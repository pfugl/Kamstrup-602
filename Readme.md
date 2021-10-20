Project to get data from Kamstrup Multical 602 heat meter, and publish some of the data to the MQTT platform.
The MQTT data is then used in Node Red to create HA sensors in order to get tha data into Home Assistant.
It is based on hardware and software from https://wiki.hal9k.dk/projects/pkamstrup.
Modified the original code to use an ESP32 instead of Arduino, and using the Microsoft Visual Studio  Platformio extension to compile the project.
The ESP32 Serial2 uart pins 16 (rcv) and 17 (tx) are used for serial communication with the IR eye.
Added code to communicate with MQTT broker over WiFi.
Before compiling:

1.  Add the PubSubClient library to the project
2.  Rename credentials.h.exampel to credentials.h 
3.  Insert the needed information for WiFi, and MQTT broker credentials into the credentials.h file.

This changed version and the original version is licensed under the folllowing license:  
 https://creativecommons.org/licenses/by-sa/4.0/