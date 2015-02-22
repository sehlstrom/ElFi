/**
 * @flie elfi.ino
 * @version 1.1
 *
 * @section License
 * Copyright (C) 2015,      Alexander Sehlström (ElFi implementation)
 * Copyright (C) 2013-2015, Mikael Patel (Cosa libarry and related examples)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * @section Description
 * ElFi is a Do It Yourself Home Automation System.
 *
 * ElFi controlls a set of NEXA Switches through a RF433 transmitter.
 * Controll can be automated using activities, e.g. switch on all
 * NEXA Switches in the morning, or manually using the HTTP interface
 * and e.g. a smart phone. In the later case, a Arduino Ethernet Sheild
 * (or a WiFi Sheild) is needed. If internet access is provided for ElFi,
 * the real time clock (RTC) of ElFi is set by retreiving the time form
 * an NTP once a day. Otherwise the RTC has to be set manually before any
 * command is sent to ElFi in order to make the activities run at the
 * correct time.
 *
 * To reduce unnecessary memory usage, it is essential that ElFi is
 * configured properly at compile time. This is done by defining a few
 * values prior to including ELFI.h:
 * - NEXA_SWITCHES      The number of NEXA switches that ElFi shall
 *                      controll. Default and maximum is 16.
 * - NEXA_ACTIVITIES    The number of NEXA activities to use. Each
 *                      Activity is rather memory consuming so choose
 *                      the number wisely.
 * - NTP_TIME_ZONE      Offset from GMT. Default is 1.
 * - NTP_SERVER         NTP server to use. Default is "se.pool.ntp.org"
 *
 * In order for ElFi to work, you need:
 * - Arduino with Ethernet Sheild (or WiFi sheild)
 * - RF433/TX
 * - NEXA Self learning switches, e.g. NEXA NEYC-3
 * - Cosa library (Object-Oriented Platform for Arduino Programming,
 *   https://github.com/mikaelpatel/Cosa)
 * - Dedicated LAN IP for the Ethernet Sheild to allow easy acces
 *   to the web server on the Arduino
 *
 * @section Circuit
 * This sketch is designed for the Ethernet Shield wired with a
 * RF433/TX.
 *
 * @code
 *                       W5100/ethernet
 *                       +------------+
 * (D10)--------------29-|CSN         |
 * (D11)--------------28-|MOSI        |
 * (D12)--------------27-|MISO        |
 * (D13)--------------30-|SCK         |
 * (D2)-----[ ]-------56-|IRQ         |
 *                       +------------+
 *
 *                         RF433/TX
 *                       +------------+
 * (GND)---------------1-|GND         |
 * (D9)----------------2-|DATA IN     |                   V
 * (VCC)---------------3-|VCC         |                   |
 *                       |ANT       0-|-------------------+
 *                       +------------+       173 mm
 * @endcode
 *
 * Connect the Ethernet Shield on the Arduino and then the
 * Ethernet Sheild D9 to RF433 Transmitter data in pin.
 *
 * This file is part of the Arduino ElFi project.
 */
 
//#define NEXA_SWITCHES 3
//#define NEXA_ACTIVITIES 4

#include "ELFI.h"

#include "Cosa/Watchdog.hh"

#define USE_ETHERNET_SHIELD
#if defined(USE_ETHERNET_SHIELD)
#include "Cosa/OutputPin.hh"
OutputPin sd(Board::D4, 1);
#endif

// Ethernet
#define MAC 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed
static const uint8_t mac[6] __PROGMEM = { MAC };
W5100 ethernet(mac);

// NEXA RF433/TX transmitter.
NEXA::Transmitter transmitter(Board::D9, 0xc05a01L);

// The one and only ElFi object
ELFI elfi;

void setup() {
  Watchdog::begin(16, Watchdog::push_timeout_events);
  RTC::begin();
  
  // Activate the switches
  elfi.activate_NEXA_Switch(0, "Vardagsrumsfönstret");
  elfi.activate_NEXA_Switch(1, "Vardagsrummet");
  elfi.activate_NEXA_Switch(2, "Hallen");
  
  // Activate the activities
  elfi.activate_NEXA_Activity(0, "God morgon", WEEKDAYS, 6, 40, 1);
  elfi.activate_NEXA_Activity(1, "Dagsa att gå till jobbet", WEEKDAYS, 7, 25, 0);
  elfi.activate_NEXA_Activity(2, "Dags att sova", BEFOREWORKDAY, 22, 40, 0);
  elfi.activate_NEXA_Activity(3, "God morgon", WEEKENDDAYS, 8, 30, 1);
  
  // Start ElFi with a transmitter, ethernet connection and use HTTP access
  elfi.begin(&transmitter, &ethernet, true);
}

void loop() {
  elfi.run();
}
