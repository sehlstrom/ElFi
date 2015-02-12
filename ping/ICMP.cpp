/*
 * @flie ICMP.h
 * @version 0.1
 *
 * @section License
 * Copyright (C) 2015,      Alexander Sehlström
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
 * This file is part of the Arduino ElFi project.
 * The file is under development and has not been tested.
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
 
#include "ICMP.h"

/**
 * Make a 16-bit unsigned integer given the low order and high order bytes.
 * lowOrder first because the Arduino is little endian.
 *
 * @section Acknowledgements
 * Conversion code by Blake Foster
 *
 * @section References
 * 1. ICMPPing.cpp
 * https://github.com/BlakeFoster/Arduino-Ping/tree/master/icmp_ping
 * 
 * @param[in] highOrder
 * @param[in] lowOrder
 * @return 16-bit unsigned integer
 */
inline uint16_t
makeUint16(const uint8_t& highOrder, const uint8_t& lowOrder)
{
    uint8_t value [] = {lowOrder, highOrder};
    return *(uint16_t *)&value;
}

ICMP::ICMP() :
  m_sock(NULL)
{
}

// Should return something more useful
int
ICMP::ping(uint8_t ip[4], uint8_t retries)
{  
  return send(ip, ICMP_ECHO_REQUEST);
  
  return recv(ip, ICMP_ECHO_REQUEST);
}

bool
ICMP::begin(Socket* sock)
{
  if (m_sock != NULL) return (false);
  m_sock = sock;
  return (true);
}

int
ICMP::send(uint8_t ip[4], uint8_t type, uint8_t code)
{
  // Check if we have a socket
  if (m_sock == NULL) return (-2);
  
  #if defined(DEVMODE)
  trace << PSTR("Socket protocol ") << m_sock->get_proto() << endl
        << PSTR("Socket port ") << m_sock->get_port() << endl;
  #endif
  
  // Start constructing the message
  message_t msg;
  header_t header;
  
  // Start the construction of the message
  msg.ID = (uint16_t)random();
  msg.SEQ = m_nextSeq++;
  msg.TIME = RTC::time();
  msg.HEADER = header;
  
  // Construct the message header
  memset(&header, 0, sizeof(header));
  header.TYPE = type;
  header.CODE = code;
  header.CHECKSUM = INET::checksum(&msg, sizeof(msg));
  int res = m_sock->send(&header, sizeof(header), ip, 0, false);    // NOT WORKING: FUNCTION IS PROTECTED!
  if (res < 0) return (-1);
  
  return (m_sock->flush());
}

// Parse return message needs to be added
int
ICMP::recv(uint8_t ip[4], uint8_t type, uint8_t code, uint16_t ms)
{
  // Wait for a reply
  int res = 0;
  for (uint16_t i = 0; i < ms; i += 32) {
    if ((res = m_sock->available()) != 0) break;
    delay(32);
  }
  if (res == 0) return (-1);
  
  // Read response message
  header_t header;
  res = m_sock->recv(&header, sizeof(header));
  if (res <= 0) return (-2);
  
  // Parse options
  
  // Flush any remains of the reply
  
  // Workaround to make code compile until function is working.
  return -10;
}
