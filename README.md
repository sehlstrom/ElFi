# ElFi
ElFi is a Do It Yourself Home Automation System.

ElFi handels  divece timers that, using a RF433 transmitter, switches devices on/off at given time at specified days. Furthermore, using the Arduino Ethernet Sheild that retreives a LAN IP from a router using DHCP, ElFi hosts a web server. The web server provides an HTML interface that allows manual controll of the connected devices using e.g. your smart phone. Also, the real time clock of ElFi is set by retreiving the time from a NTP.

## Requirements
In order for ElFi to work, you need:
* Arduino Uno
* Arduino Ethernet Sheild
* RF433/TX
* NEXA Self learning switches, e.g. NEXA NEYC-3
* COSA library (Object-Oriented Platform for Arduino Programming, https://github.com/mikaelpatel/Cosa)
* Dedicated LAN IP for the Ethernet Sheild to allow easy acces to the web server on the Arduino
* RJ45 cable

## Setup
- Connect everything: Attach the Ethernet Sheild to the Arduino and connect the RF433/TX according to the specific component. Plug in an RJ45 cable in your router and in the sheild.
- Download the COSA library and install according to the COSA instructions.
- In the Arduino program choose from the menue Tools > Board and select the Cosa Arduino board you have.
- Pair your NEXA devices with the Arduino (see the COSA example NEXA/CosaNEXAsender.ino) giving your devices numbers ranging from 0 to 15.
- Modify the elfi.ino file to suit your needs, e.g. adding/removing devices, set up device timers etc. Add the ElFi header files to the Arduino Sketch (you can use drag-n-drop).
- Compile and upload to your Arduino.
- In your router, dedicate an LAN IP adress to the Arduino e.g. by using the MAC adress of the Arduino Ethernet Sheild. (In my case the Arduino listens to the dedicated LAN IP 10.0.1.190.)
- Open web browser and write the dedicated IP address (10.0.1.190) to access the web page hosted by ElFi.

Good luck!
