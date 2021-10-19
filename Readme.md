Project to get data from Kamstrup 602 heat meter, and publish some of the data to the MQTT platform.
It is based on hardware and software from https://wiki.hal9k.dk/projects/kamstrup.
Modified the original code to use an  ESP32 with the Microsoft Visual Studio  Platformio extension.
Added code to communicate with MQTT and WiFi.
Before compiling, rename credentials.h.exampel to credentials.h and insert the needed information for WiFi, and MQTT credentials in the credentials.h file.