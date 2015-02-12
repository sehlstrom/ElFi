/**
 * @flie ping.ino
 * @version 1.0
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
 * The file ping.ino tests the ICMP ping functions.
 *
 * @section Circuit
 * This sketch is designed for the Ethernet Shield
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
 * @endcode
 *
 * Connect the Ethernet Shield on the Arduino and then the
 * Ethernet Sheild D9 to RF433 Transmitter data in pin.
 *
 * This file is part of the Arduino ElFi project.
 */

// DEVELOPMENT MODE ============================================================
//#define DEVMODE 1

#if defined DEVMODE
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#endif

// INCLUDES ====================================================================
// Cosa library ----------------------------------------------------------------
#include "Cosa/INET/DHCP.hh"
#include "Cosa/Socket/Driver/W5100.hh"

// ElFi library ----------------------------------------------------------------
#include "ICMP.h"

// DEFINITIONS =================================================================
// Network configuration -------------------------------------------------------
#define MAC 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed

// HARDWARE SETUP ==============================================================
// Ethernet Sheild -------------------------------------------------------------
// Disable SD on Ethernet Shield
#define USE_ETHERNET_SHIELD
#define USE_ETHERNET_SHIELD
#if defined(USE_ETHERNET_SHIELD)
#include "Cosa/OutputPin.hh"
OutputPin sd(Board::D4, 1);
#endif

// DECLARATIONS ================================================================
// DHCP, ethernet and web server -----------------------------------------------
// MAC-address
static const uint8_t mac[6] __PROGMEM = { MAC };

// Host name
static const char hostname[] __PROGMEM = "ElFi";

// DHCP
DHCP dhcp(hostname, mac);

// Ethernet
W5100 ethernet(mac);

// MAIN PROGRAM ================================================================
void setup()
{
 #if defined(DEVMODE)
 uart.begin(9600);
 trace.begin(&uart, PSTR("ElFi: start"));
 #endif
  
  // Initiate the Ethernet Controller using DHCP
  #if defined(DEVMODE)
  ASSERT(ethernet.begin_P(PSTR("ElFi")));
  #else
  ethernet.begin_P(PSTR("ElFi"));
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  
  ICMP::ping( , );

}
