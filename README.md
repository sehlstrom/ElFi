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
