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
// Libraries neede to be able to print messages to terminal. No need to include
// these if we are not currently developing the code since no terminal will be
// connected during normal usage.
#define DEVMODE 1

#if defined DEVMODE
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#endif

// INCLUDES ====================================================================
// Cosa library ----------------------------------------------------------------
#include "Cosa/INET/DNS.hh"
#include "Cosa/INET/NTP.hh"
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

// Ethernet
W5100 ethernet(mac);

// ICMP ------------------------------------------------------------------------
// ICMP
ICMP icmp;

// IP address to ping
uint8_t pingIp[] = { 10 , 0 , 1 , 175 }; // 102 computer, 175 phone

// Number of retries
uint8_t pingTimeout = 4;

// Time ------------------------------------------------------------------------
#define TIME_ZONE 1                              // Time-zone; GMT+1, Stockholm
#define TIME_NTP_SERVER "se.pool.ntp.org"        // NTP server; se.pool.ntp.org

// FUNCTIONS ===================================================================
// Time functions --------------------------------------------------------------
/**
 * Update the Real Time Clock on the Arduino. Also updates the time in all the
 * alarms and activities.
 */
void
update_RTC()
{
  // Update the RTC
  RTC::time(get_NTP_time());
}

/**
 * Get the current time from a NTP.
 *
 * @section Acknowledgements
 * The function is based on the Cosa NTP example related to the Cosa
 * library by Mikael Patel. See references for more details.
 *
 * @section References
 * 1. CosaNTP.ino example file.
 * https://github.com/mikaelpatel/Cosa/tree/master/examples/Time/CosaNTP
 */
clock_t
get_NTP_time()
{
  uint8_t server[4];

  // Use DNS to get the NTP server network address
  DNS dns;
  ethernet.get_dns_addr(server);
  if (!dns.begin(ethernet.socket(Socket::UDP), server)) return 0L;
  if (dns.gethostbyname_P(PSTR(TIME_NTP_SERVER), server) != 0) return 0L;

  // Connect to the NTP server using given socket
  NTP ntp(ethernet.socket(Socket::UDP), server, TIME_ZONE);

  // Get current time. Allow a number of retries
  const uint8_t RETRY_MAX = 20;
  clock_t clock;
  for (uint8_t retry = 0; retry < RETRY_MAX; retry++)
    if ((clock = ntp.time()) != 0L) break;
  #if defined(DEVMODE)
  ASSERT(clock != 0L);
  #endif

  return clock;
}

// MAIN PROGRAM ================================================================
void setup()
{
  #if defined(DEVMODE)
  uart.begin(9600);
  trace.begin(&uart, PSTR("ElFi: start"));
  #endif
  
  // Start the watchdog, real-time clock and the alarm scheduler
  RTC::begin();
  
  // Initiate the Ethernet Controller using DHCP
  #if defined(DEVMODE)
  ASSERT(ethernet.begin_P(hostname));
  #else
  ethernet.begin_P(hostname);
  #endif
  
  // Clock settings
  time_t::epoch_year( NTP_EPOCH_YEAR );
  time_t::epoch_weekday = NTP_EPOCH_WEEKDAY;
  time_t::pivot_year = 37; // 1937..2036 range

  // Set the clock
  update_RTC();
  
  #if defined(DEVMODE)
  // Print current time
  time_t time = RTC::time();
  trace << PSTR("Current time ") << time << endl;
  #endif
}

void loop()
{
  int res = icmp.ping(ethernet.socket(Socket::IPRAW), pingIp);
  
  #if defined(DEVMODE)
  trace << PSTR("Result ") << res << endl;
  #endif
  
  delay( 3000 ) ;
}
