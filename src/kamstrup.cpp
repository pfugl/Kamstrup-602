
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "kamstrup.h"
#include "credentials.h"


/*********** Kamstrup Multical 602  ***********/



WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


short kregnums[] = { 0x003C,0x0050,0x0056,0x0057,0x0059,0x004a,0x0044 };                                                   // The registers we want to get out of the meter
const char* kregstrings[]   = { "Energy","Current Power","Temperature t1","Temperature t2","Temperature diff", "Flow", "Volumen 1" }; // The name of the registers we want to get out of the meter in the same order as above
#define NUMREGS 7                                                                                                               // Number of registers above
#define KAMBAUD 1200 // Serial com speed for Kamstrup 602 (8N2)

// Units
const char*  units[65] = {"","Wh","kWh","MWh","GWh","j","kj","Mj",
  "Gj","Cal","kCal","Mcal","Gcal","varh","kvarh","Mvarh","Gvarh",
        "VAh","kVAh","MVAh","GVAh","kW","kW","MW","GW","kvar","kvar","Mvar",
        "Gvar","VA","kVA","MVA","GVA","V","A","kV","kA","C","K","l","m3",
        "l/h","m3/h","m3xC","ton","ton/h","h","hh:mm:ss","yy:mm:dd","yyyy:mm:dd",
        "mm:dd","","bar","RTC","ASCII","m3 x 10","ton xr 10","GJ x 10","minutes","Bitfield",
        "s","ms","days","RTC-Q","Datetime"};

// Pin definitions for ESP32 Serial2 uart.
#define PIN_KAMSER_RX  16  // Kamstrup IR interface RX
#define PIN_KAMSER_TX  17  // Kamstrup IR interface TX

// Kamstrup optical IR serial
#define KAMTIMEOUT 300     // Kamstrup timeout after transmit

// kamReadReg - read a Kamstrup register
float kamReadReg(unsigned short kreg) {

  byte recvmsg[40];  // buffer of bytes to hold the received data
  float rval;        // this will hold the final value

  // prepare message to send and send it
  byte sendmsg[] = { 0x3f, 0x10, 0x01,(byte) (kregnums[kreg] >> 8),(byte) (kregnums[kreg] & 0xff) };
  kamSend(sendmsg, 5);

  // listen if we get an answer
  unsigned short rxnum = kamReceive(recvmsg);

  // check if number of received bytes > 0 
  if(rxnum != 0){
    
    // decode the received message
    rval = kamDecode(kreg,recvmsg);
    
    // print out received value to terminal (debug)
    Serial.print(kregstrings[kreg]);
    Serial.print(": ");
    Serial.print(rval);
    Serial.print(" ");
    Serial.println();
    
    char tempString[50];
    String test[30];
    
    // Send Energy, Temperature in, Temperature out and Temperature difference to MQTT
    switch (kreg) {
      case 0:  // energi
        dtostrf(rval, 1, 3, tempString);
        client.publish("esp32/energy", tempString);
        break;
      case 2: // Temperatur ind
        dtostrf(rval, 1, 1, tempString);
        client.publish("esp32/tempin", tempString);
        break;
      case 3: // Temperatur ud
        dtostrf(rval, 1, 1, tempString);
        client.publish("esp32/tempout", tempString);
      case 4: // Temperatur diff
        dtostrf(rval, 1, 1, tempString);
        client.publish("esp32/tempdiff", tempString);
        break;
      default:
        break;
    }
    
    return rval;
  }
return 0;
}

// kamSend - send data to Kamstrup meter
void kamSend(byte const *msg, int msgsize) {

  // append checksum bytes to message
  byte newmsg[msgsize+2];
  for (int i = 0; i < msgsize; i++) { newmsg[i] = msg[i]; }
  newmsg[msgsize++] = 0x00;
  newmsg[msgsize++] = 0x00;
  int c = crc_1021(newmsg, msgsize);
  newmsg[msgsize-2] = (c >> 8);
  newmsg[msgsize-1] = c & 0xff;

  // Build final transmit message - escape various bytes:
  // if one of following reserved bytes exist in the buffer: 06,1b,40,80, replace it with 2 bytes:
  // 0x1b, plus one complement of the byte.
  byte txmsg[20] = { 0x80 };   // prefix
  int txsize = 1;
  for (int i = 0; i < msgsize; i++) {
    if (newmsg[i] == 0x06 or newmsg[i] == 0x0d or newmsg[i] == 0x1b or newmsg[i] == 0x40 or newmsg[i] == 0x80) {
      txmsg[txsize++] = 0x1b;
      txmsg[txsize++] = newmsg[i] ^ 0xff;
    } else {
      txmsg[txsize++] = newmsg[i];
    }
  }
  txmsg[txsize++] = 0x0d;  // EOF

  // send to serial interface
  for (int x = 0; x < txsize; x++) {
    Serial2.write(txmsg[x]);
  }

}

// kamReceive - receive bytes from Kamstrup meter
unsigned short kamReceive(byte recvmsg[]) {

  byte rxdata[50];  // buffer to hold received data
  unsigned long rxindex = 0;
  unsigned long starttime = millis();
  
  Serial2.flush();  // flush serial buffer - might contain noise

  byte r=0;
  
  // loop until EOL received or timeout
  while(r != 0x0d){
    
    // handle rx timeout
    if(millis()-starttime > KAMTIMEOUT) {
      Serial.println("Timed out listening for data");
      client.publish("esp32/timeout", "Timeout");
      return 0;
    }

    // handle incoming data
    if (Serial2.available()) {

      // receive byte
      r = Serial2.read();
      if(r != 0x40) {  // don't append if we see the start marker
        // append data
        rxdata[rxindex] = r;
        rxindex++; 
      }

    }
  }

  // remove escape markers from received data
  unsigned short j = 0;
  for (unsigned short i = 0; i < rxindex -1; i++) {
    if (rxdata[i] == 0x1b) {
      byte v = rxdata[i+1] ^ 0xff;
      if (v != 0x06 and v != 0x0d and v != 0x1b and v != 0x40 and v != 0x80){
        Serial.print("Missing escape ");
        Serial.println(v,HEX);
      }
      recvmsg[j] = v;
      i++; // skip
    } else {
      recvmsg[j] = rxdata[i];
    }
    j++;
  }
  
  // check CRC
  if (crc_1021(recvmsg,j)) {
    Serial.println("CRC error: ");
    return 0;
  }
  
  return j;
  
}

// kamDecode - decodes received data
float kamDecode(unsigned short const kreg, byte const *msg) {

  // skip if message is not valid
  if (msg[0] != 0x3f or msg[1] != 0x10) {
    return false;
  }
  if (msg[2] != (kregnums[kreg] >> 8) or msg[3] != (kregnums[kreg] & 0xff)) {
    return false;
  }
    
  // decode the mantissa
  long x = 0;
  for (int i = 0; i < msg[5]; i++) {
    x <<= 8;
    x |= msg[i + 7];
  }
  
  // decode the exponent
  int i = msg[6] & 0x3f;
  if (msg[6] & 0x40) {
    i = -i;
  };
  float ifl = pow(10,i);
  if (msg[6] & 0x80) {
    ifl = -ifl;
  }

  // return final value
  return (float )(x * ifl);

}

// crc_1021 - calculate crc16
long crc_1021(byte const *inmsg, unsigned int len){
  long creg = 0x0000;
  for(unsigned int i = 0; i < len; i++) {
    int mask = 0x80;
    while(mask > 0) {
      creg <<= 1;
      if (inmsg[i] & mask){
        creg |= 1;
      }
      mask>>=1;
      if (creg & 0x10000) {
        creg &= 0xffff;
        creg ^= 0x1021;
      }
    }
  }
  return creg;
}

// Initialize WiFi, and connect.
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Callback routine to receive incomming MQTT requests.
// Presently not used for anything.
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

// Connect to MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mquser, mqpasswd)) {
      Serial.println("MQTT_connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// This is where execution starts:
void setup () {
  Serial.begin(115200);
  Serial.println("BOOT");

  
  // setup kamstrup serial
  pinMode(PIN_KAMSER_RX,INPUT);
  pinMode(PIN_KAMSER_TX,OUTPUT);
  Serial2.begin(KAMBAUD,SERIAL_8N2);

  // Start wifi, and connect to mqtt broker
  initWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  delay(200);
  
  Serial.println("\n[testKamstrup]");
  // poll the Kamstrup registers for data 
    for (int kreg = 0; kreg < NUMREGS; kreg++) {
    //  kamReadReg(kreg);
      delay(100);
  }
}

// Main execution loop
void loop () {
  // check mqtt connected:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Check if anything is received from mqtt.

  // poll the Kamstrup registers for data 
  for (int kreg = 0; kreg < NUMREGS; kreg++) {
    kamReadReg(kreg);
    delay(100);
  }
  
  // Wait 5 seconds between cycles. If set much higher (f.ex 15000), it may cause MQTT timeouts.
  delay(5000);
}

