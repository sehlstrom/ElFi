/**
 * @flie elfi.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2015, Alexander Sehlström
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
 * ElFi handels  divece timers that, using a RF433 transmitter,
 * switches devices on/off at given time at specified days.
 * Furthermore, using the Arduino Ethernet Sheild that retreives a
 * LAN IP from a router using DHCP, ElFi hosts a web server. The web
 * server provides an HTML interface that allows manual controll of
 * the connected devices using e.g. your smart phone. Also, the real
 * time clock of ElFi is set by retreiving the time from a NTP.
 *
 * In order for ElFi to work, you need:
 * - Arduino with Ethernet Sheild
 * - RF433/TX
 * - NEXA Self learning switches, e.g. NEXA NEYC-3
 * - COSA library (Object-Oriented Platform for Arduino Programming,
 *   https://github.com/mikaelpatel/Cosa)
 * - Dedicated LAN IP for the Ethernet Sheild to allow easy acces
 *   to the web server on the Arduino
 *
 * @section Circuit
 * This sketch is designed for the Ethernet Shield wired
 * with a RF433/TX.
 *
 * 
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
#include "Cosa/Trace.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/OutputPin.hh"

#include "Cosa/INET/DHCP.hh"
#include "Cosa/INET/DNS.hh"
#include "Cosa/INET/NTP.hh"

#include "Cosa/Driver/NEXA.hh"

#include "Cosa/Socket/Driver/W5100.hh"

// ElFi library ----------------------------------------------------------------
#include "TimerActivity.h"
#include "WebServer.h"

// DEFINITIONS =================================================================
// Network configuration -------------------------------------------------------
#define MAC 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed
#define PORT 80

// Time ------------------------------------------------------------------------
#define TIME_ZONE 1                              // Time-zone; GMT+1, Stockholm
#define TIME_NTP_SERVER "se.pool.ntp.org"        // NTP server; se.pool.ntp.org

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
// Alarm and activity ----------------------------------------------------------
// Alarm and activity sheduler
Alarm::Scheduler scheduler;

// DeviceTimer activities
const uint8_t allDays[7] = {1, 1, 1, 1, 1, 1, 1};
const uint8_t weekDays[7] = {0, 1, 1, 1, 1, 1, 0};
const uint8_t weekendDays[7] = {1, 0, 0, 0, 0, 0, 1};

const uint8_t all = -1;

const uint8_t on = 1;
const uint8_t off = 0;

FunctionTimer functionTimers[1] = {
  FunctionTimer(weekDays,   6, 40,  0, &update_RTC),   //  1 Update the RTC
};

DeviceTimer deviceTimers[5] = {
  DeviceTimer(weekDays,     6, 40,  0, all, on),       //  1 Weekday morning on
  DeviceTimer(weekDays,     7, 25,  0, all, off),      //  2 Weekday morning off
  DeviceTimer(weekDays,    22, 30,  0, all, off),      //  3 Weekday night off
  DeviceTimer(weekendDays,  8, 30,  0, all, on),       //  4 Weekend morning on
  DeviceTimer(allDays,     23, 30,  0, all, off)       //  5 All days night off
};

// DHCP, ethernet and web server -----------------------------------------------
// MAC-address
static const uint8_t mac[6] __PROGMEM = { MAC };

// Host name
static const char hostname[] __PROGMEM = "ElFi";

// DHCP
DHCP dhcp(hostname, mac);

// Ethernet
W5100 ethernet(mac);

// Web server
WebServer server;

// HTML end of line
#define CRLF "\r\n"

// NEXA communications ---------------------------------------------------------
// NEXA RF433/TX transmitter.
NEXA::Transmitter transmitter(Board::D9, 0xc05a01L);

// CLASS IMPLEMENTATAIONS ======================================================
// WebServer -------------------------------------------------------------------
/**
 * The HTML page provided on request is Apple Web Application compatible. It
 * uses a simple jQuery script to pass background GET queries triggerd by the
 * interface buttons. The interface design is controlled via a header defined
 * style (CSS).
 */
void 
WebServer::on_request(IOStream& page, char* method, char* path, char* query)
{  
  static const char header[] __PROGMEM = 
    "HTTP/1.1 200 OK" CRLF
    "Content-Type: text/html" CRLF
    "Connection: close" CRLF CRLF
    "<!DOCTYPE HTML>" CRLF
    "<html>" CRLF
    "<head>" CRLF
    "<meta charset=\"UTF-8\">"
    "<meta name='apple-mobile-web-app-capable' content='yes' />" CRLF
    "<meta name='apple-mobile-web-app-status-bar-style' content='black' />" CRLF
    "<meta name='apple-mobile-web-app-title' content='Home Automation System' />" CRLF
    "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable = no'>" CRLF
    "<script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js\"></script>" CRLF
    "<script>" CRLF
    "  function deviceControll(url) {$.ajax(url);}" CRLF
    "</script>" CRLF
    "<style>" CRLF
    "body{margin:0; font-family:Helvetica,Arial,Sans-Serif; font-size:14px; background:#CCC; box-sizing:border-box;}" CRLF
    "*, *:before, *:after {box-sizing: inherit;}" CRLF
    "h1,h2,h3{display: block; padding:8px; margin:0;}" CRLF
    "h1{background-color:#67D66F; color: white;}" CRLF
    "h2, h3 {padding-left:0px;}" CRLF
    ".device{display:table; width:100%; height:20px; background:white; padding:8px; margin-bottom:1px}" CRLF
    ".device-lable{display:table-cell; width:60%; height:inherit; vertical-align:middle;}" CRLF
    ".device-button{display:table-cell; width:20%; height:inherit; vertical-align:middle; text-align:center;}" CRLF
    ".devices .device:first-child{background-color:#9CEF9F; color: white; margin-bottom:0;}" CRLF
    "button{background:#67D66F; padding:5px 10px; margin: auto; border:hidden; -webkit-border-radius:3px; -moz-border-radius:3px; border-radius:3px; color:white; vertical-align:middle; min-width:95%;}" CRLF
    "div#time{color:gray; padding:8px; font-size:10px}" CRLF
    "</style>" CRLF
    "<title>ElFI - Home Automation System</title>" CRLF
    "</head>" CRLF 
    "<body>" CRLF;
    
  static const char body1[] __PROGMEM = 
    "<h1>ElFi</h1>" CRLF
    "<div class=\"devices\">" CRLF
    "<div class=\"device\">" CRLF
    "<div class=\"device-lable\"><h2>NEXA Devices</h2></div>" CRLF
    "<div class=\"device-button\">" CRLF
    "<button onclick=\"deviceControll('http://10.0.1.190/?all_devices_on');\">All on</button>" CRLF
    "</div>" CRLF
    "<div class=\"device-button\">" CRLF
    "<button onclick=\"deviceControll('http://10.0.1.190/?all_devices_off');\">All off</button>" CRLF
    "</div>" CRLF
    "</div>" CRLF;
    
  static const char body_device1[] __PROGMEM = 
    "<div class=\"device\">" CRLF
    "<div class=\"device-lable\"><h3>NEXA ";
    
  static const char body_device2[] __PROGMEM = 
    "</h3></div>" CRLF
    "<div class=\"device-button\">" CRLF
    "<button onclick=\"deviceControll('http://10.0.1.190/?device_on_";
    
  static const char body_device3[] __PROGMEM = 
    "');\">On</button>" CRLF
    "</div>" CRLF
    "<div class=\"device-button\">" CRLF
    "<button onclick=\"deviceControll('http://10.0.1.190/?device_off_";
    
  static const char body_device4[] __PROGMEM = 
    "');\">Off</button>" CRLF
    "</div>" CRLF
    "</div>" CRLF;
    
  static const char footer[] __PROGMEM = 
    "</body>" CRLF 
    "</html>";
  
  // Construct the page; header-contents-footer
  page << (str_P) header;
  page << (str_P) body1;
    
  // All unique devices
  for (int i = 0; i < 3; i++)
  {
    page << (str_P) body_device1
         << i
         << (str_P) body_device2
         << i
         << (str_P) body_device3
         << i
         << (str_P) body_device4;
  }
  
  page << PSTR("</div>") << endl;
  
  time_t time = RTC::time();
  page << PSTR("<div id =\"time\">Time: ") << time << PSTR("</div>") << endl;
  
  page << (str_P) footer;
  
  // Handle queries
  if (query != NULL)
  {
    const char *function;
   
    function = "all_devices_on";
    if(strcmp(query, function) == 0) {
      transmitter.broadcast(0, on);
      transmitter.broadcast(1, on);
      transmitter.broadcast(2, on);
      transmitter.broadcast(3, on);
    }
    
    function = "all_devices_off";
    if(strcmp(query, function) == 0) {
      transmitter.broadcast(0, off);
      transmitter.broadcast(1, off);
      transmitter.broadcast(2, off);
      transmitter.broadcast(3, off);
    }
    
    function = "device_on_0";
    if(strcmp(query, function) == 0) {
      transmitter.send(0, on);
    }
    
    function = "device_off_0";
    if(strcmp(query, function) == 0) {
      transmitter.send(0, off);
    }
      
      function = "device_on_1";
    if(strcmp(query, function) == 0) {
      transmitter.send(1, on);
    }
    
    function = "device_off_1";
    if(strcmp(query, function) == 0) {
      transmitter.send(1, off);
    }
    
    function = "device_on_2";
    if(strcmp(query, function) == 0) {
      transmitter.send(2, on);
    }
    
    function = "device_off_2";
    if(strcmp(query, function) == 0) {
      transmitter.send(2, off);
    }
  }  
}

// DeviceTimer -----------------------------------------------------------------
void
DeviceTimer::enable()
{
  // Set time of the activity
  time_t start = RTC::time();
  start.hours = m_hours;
  start.minutes = m_minutes;
  start.seconds = m_seconds;
  
  // Pass first time (in seconds after epoch time) for the first occurence of the activity
  clock_t start_clock = start;
  set_time(start_clock, 1, 1440);
  
  // Enable the activity
  Activity::enable();
  
  #if defined(DEVMODE)
  trace << PSTR("Enables DeviceTimer at ") << start 
        << PSTR(" (") << start_clock << PSTR(" seconds)")
        << endl;
  #endif
}

void
DeviceTimer::run()
{
  #if defined(DEVMODE)
  trace << PSTR("DeviceTimer dispatched: ") << endl;
  #endif
  time_t time = RTC::time();
  uint8_t d = time.day;
  if (m_day[d-1]) {
    if (m_device != 255)
    {
      transmitter.send(m_device, m_mode);
    }
    else
    {
      transmitter.broadcast(0, m_mode);
      transmitter.broadcast(1, m_mode);
      transmitter.broadcast(2, m_mode);
      transmitter.broadcast(3, m_mode);
    }
  }
}

// Function timer --------------------------------------------------------------
void
FunctionTimer::enable()
{
  // Set time of the activity
  time_t start = RTC::time();
  start.hours = m_hours;
  start.minutes = m_minutes;
  start.seconds = m_seconds;
  
  // Pass first time (in seconds after epoch time) for the first occurence of the activity
  clock_t start_clock = start;
  set_time(start_clock, 1, 1440);
  
  // Enable the activity
  Activity::enable();
  
  #if defined(DEVMODE)
  trace << PSTR("Enables FunctionTimer at ") << start 
        << PSTR(" (") << start_clock << PSTR(" seconds)")
        << endl;
  #endif
}

void
FunctionTimer::run()
{
  #if defined(DEVMODE)
  trace << PSTR("FunctionTimer dispatched: ") << endl;
  #endif
  time_t time = RTC::time();
  uint8_t d = time.day;
  if (m_day[d-1]) {
    m_function();
  }
}

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

  // Match time in Alarm with RTC
  Alarm::set_time(RTC::time());
}

/**
 * Get the current time from a NTP.
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
  ASSERT(clock != 0L);

  return clock;
}

// MAIN PROGRAM ================================================================
void
setup()
{
  #if defined(DEVMODE)
  uart.begin(9600);
  trace.begin(&uart, PSTR("ElFi: start"));
  #endif
  
  // Start the watchdog, real-time clock and the alarm scheduler
  Watchdog::begin(16, Watchdog::push_timeout_events);
  RTC::begin();
  scheduler.begin();

  // Initiate the Ethernet Controller using DHCP
  ASSERT(ethernet.begin_P(PSTR("ElFi")));
  
  // Start the server
  ASSERT(server.begin(ethernet.socket(Socket::TCP, PORT)));

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

  // Enable the timer handlers
  int m = sizeof(functionTimers)/sizeof(*functionTimers);
  for (int i = 0; i < m; i++) {
    functionTimers[i].enable();
  }
  
  m = sizeof(deviceTimers)/sizeof(*deviceTimers);
  for (int i = 0; i < m; i++) {
    deviceTimers[i].enable();
  }
}

void loop()
{
  // The standard event dispatcher
  Event event;
  while (Event::queue.dequeue( &event ))
    event.dispatch();
  
  // Service incoming requests
  server.run(5L);
}
