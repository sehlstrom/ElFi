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

// This shoudl not be void, it should return some useful information
void
ICMP::ping(uint8_t ip[4], uint8_t retries)
{
  send(ip, (uint16_t)random(), ICMP_ECHO_REQUEST, 0);
}

// The function needs development. The IP address needs to be passed and a connection established. 
int
ICMP::send(uint8_t ip[4], uint8_t id, uint8_t type, uint8_t code) {
  message_t msg;
  header_t header;
  
  // Start the construction of the message
  msg.ID = id;
  msg.SEQ = m_nextSeq++;
  msg.TIME = RTC::time();
  msg.HEADER = header;
  
  // Construct the message header
  memset(&header, 0, sizeof(header));
  header.TYPE = type;
  header.CODE = code;
  header.CHECKSUM = INET::checksum(&msg, sizeof(msg));
  int res = m_sock->write(&header, sizeof(header));
  if (res < 0) return (-1);
  
  return (m_sock->flush());
}
