Project to get data from Kamstrup Multical 602 heat meter, and publish some of the data to the MQTT platform.
The MQTT data is then used in Node Red to create HA sensors in order to get tha data into Home Assistant.
It is based on hardware and software from https://wiki.hal9k.dk/projects/kamstrup.
Modified the original code to use an ESP32 instead of Arduino, and using the Microsoft Visual Studio  PlatformIO extension to compile the project.
Added possibility to update the code over WiFi (ArduinoOTA)
The ESP32 Serial2 uart pins 16 (rcv) and 17 (tx) are used for serial communication with the IR eye.
Added code to communicate with MQTT broker over WiFi.
Before compiling:

1.  Add the PubSubClient MQTT library to the project
2.  Rename credentials.h.exampel to credentials.h 
3.  Insert the needed information for WiFi, and MQTT broker credentials into the credentials.h file.

If you hava a problem to connect (receive timeouts), try to turn the infrared head 180 degrees clockvise and back again in order to activate the magnetic sensor in the meter. It may take a few tries to get it working. The transmit IR in the meter, is placed to the right when looking to the front of the meter.
You can also activate the meter transmission by pressing any buttom on the meter, and it will transmit for some time (5-10 minutes), and then stop if the magnetic sensor is not activated.

This changed version and the original version is licensed under the folllowing license:  
 https://creativecommons.org/licenses/by-sa/4.0/
