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

uint16_t
ICMP::checksum(const ICMP::message_t& msg)
{
    unsigned long sum = 0;

    // add the header, bytes reversed since we're using little-endian arithmetic.
    sum += makeUint16(msg.HEADER.TYPE, msg.HEADER.CODE);

    // add id and sequence
    sum += msg.ID + msg.SEQ;

    // add time, one half at a time.
    uint16_t const * time = (uint16_t const *)&msg.TIME;
    sum += *time + *(time + 1);
    
    // add the payload
    for (uint8_t const * b = msg.PAYLOAD; b < msg.PAYLOAD + sizeof(msg.PAYLOAD); b+=2)
    {
        sum += makeUint16(*b, *(b + 1));
    }

    // ones complement of ones complement sum
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
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
  header.CHECKSUM = checksum(msg);
  int res = m_sock->write(&header, sizeof(header));
  if (res < 0) return (-1);
  
  return (m_sock->flush());
}
