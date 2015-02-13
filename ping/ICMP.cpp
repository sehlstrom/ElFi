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
ICMP::ping(Socket* sock, uint8_t ip[4], uint8_t retries)
{ 
  begin(sock);
  
  int res = send(ip, ICMP_ECHO_REQUEST);
  
  res = recv(ip, ICMP_ECHO_REPLY);
  
  end();
  
  return res;
}

bool
ICMP::begin(Socket* sock)
{
  if (m_sock != NULL) return (false);
  m_sock = sock;
  return (true);
}

bool
ICMP::end()
{
  if (m_sock == NULL) return (false);
  m_sock->close();
  m_sock = NULL;
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
  
  // Construct the message
  msg.HEADER = header;
  msg.ID = (uint16_t)random();
  msg.SEQ = m_nextSeq++;
  msg.TIME = RTC::time();
  
  // Construct the message header
  memset(&header, 0, sizeof(header)); // This is necessary, but I do not know why.
  header.TYPE = type;
  header.CODE = code;
  header.CHECKSUM = INET::checksum(&msg, sizeof(msg));
  
  #if defined(DEVMODE)
  trace << PSTR("MESSAGE TO SEND (") 
        << ip[0] << "." << ip[1] << "."
        << ip[2] << "." << ip[3] << ":" 
        << PORT << ")" << endl
        << PSTR("  Header.Type: ") << msg.HEADER.TYPE << endl
        << PSTR("  Header.Code: ") << msg.HEADER.CODE << endl
        << PSTR("  Header.Checksum: ") << msg.HEADER.CHECKSUM << endl
        << PSTR("  Id: ") << msg.ID << endl
        << PSTR("  Seq: ") << msg.SEQ << endl
        << PSTR("  Time: ") << msg.TIME << endl;
  #endif
  
  // Send the message
  int res = m_sock->send(&msg, sizeof(msg), ip, PORT);
  if (res < 0) return (-1);
  
  #if defined(DEVMODE)
  trace << PSTR("MESSAGE SENT") << endl;
  #endif
  
  return (m_sock->flush());
}

// Parse return message needs to be added
int
ICMP::recv(uint8_t ip[4], uint8_t type, uint8_t code, uint16_t ms)
{
  #if defined(DEVMODE)
  trace << PSTR("WAITING FOR RESPONSE ");
  #endif
  
  int res = 0;
  
  // Wait for a reply
  for (uint16_t i = 0; i < ms; i += 32) {
    if ((res = m_sock->available()) != 0) break;
    delay(32);
    #if defined(DEVMODE)
    trace << PSTR(".");
    #endif
  }
  #if defined(DEVMODE)
  trace << endl;
  if (res == 0) {
    trace << PSTR("  No response received") << endl;
    return (-1);
  }
  trace << PSTR("  Response received") << endl;
  #else
  if (res == 0) return (-1);
  #endif
  
  // Read response message
  header_t header;
  res = m_sock->recv(&header, sizeof(header));
  if (res <= 0) return (-2);
  
  #if defined(DEVMODE)
  trace << PSTR("RESPONSE MESSAGE (")
        << ip[0] << "." << ip[1] << "."
        << ip[2] << "." << ip[3] << ":" 
        << port << ")" << endl
        << PSTR("  Header.Type: ") << msg.HEADER.TYPE << endl
        << PSTR("  Header.Code: ") << msg.HEADER.CODE << endl
        << PSTR("  Header.Checksum: ") << msg.HEADER.CHECKSUM << endl
        << PSTR("  Id: ") << msg.ID << endl
        << PSTR("  Seq: ") << msg.SEQ << endl
        << PSTR("  Time: ") << msg.TIME << endl;
  #endif
  
  // Parse options
  
  // Flush any remains of the reply
  
  // Workaround to make code compile until function is working.
  return -10;
}
