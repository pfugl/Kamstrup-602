Project to get data from Kamstrup 602 heat meter, and publish some of the data to the MQTT platform.
It is based on hardware and software from https://wiki.hal9k.dk/projects/kamstrup.
Modified the original code to use an ESP32 instead of Arduino using the Microsoft Visual Studio  Platformio extension.
The ESP32 Serial2 uart pins 16 (rcv) and 17 (tx) are used for serial communication with the IR eye.
Added code to communicate with MQTT and WiFi.
Before compiling:

1.  Add the PubSubClient library to the project
2.  Rename credentials.h.exampel to credentials.h 
3.  Insert the needed information for WiFi, and MQTT credentials into the credentials.h file.