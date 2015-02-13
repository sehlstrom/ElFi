/**
 * @file ICMP.hh
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
 
#ifndef ELFI_INET_ICMP_HH
#define ELFI_INET_ICMP_HH

#include "Cosa/INET.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Socket.hh"
#include "Cosa/Types.h"

#define REQ_DATASIZE 64

/**
 * Internet Control Message Protocol. Under development.
 *
 * @section Reference
 * 1. Wikipedia
 * http://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
 * 2. The DHCP.hh implementation in Cosa as a reference
 *    of a protocol implementation in Cosa.
 * https://github.com/mikaelpatel/Cosa
 */
class ICMP {
public:
  static const uint16_t PORT = 0;
  
  /**
   *
   */
  ICMP();
  
  /**
   * Start interaction using ICMP. Provide IPRAW socket.
   * Returns true if successful otherwise false.
   * @param[in] sock connection-less socket (IPRAW/ICMP).
   * @return bool, true if successful otherwise false.
   */
  bool begin(Socket* sock);
  
  /**
   * Stop interaction using ICMP server. Closes socket.
   * Returns true if successful otherwise false.
   */
  bool end();
  
  /**
   * Send a ping to given IP address.
   * @param[in] ip address to ping
   * @param[in] retries
   */
  int ping(Socket* sock, uint8_t ip[4], uint8_t retries = 4);
  
private:
  /** ICMP packet header. */
  struct header_t {
    uint8_t TYPE;                    //!< Message type.
    uint8_t CODE;                    //!< Message code.
    uint16_t CHECKSUM;               //!< Message checksum.
  };
  
  /** ICMP message to echo. */
  struct message_t {
    uint16_t ID;                     //!< Message id.
    uint16_t SEQ;                    //!< Message sequence number.
    time_t TIME;                     //!< Message time.
    uint8_t PAYLOAD [REQ_DATASIZE];  //!< Message payload.
    header_t HEADER;                 //!< Message header.
  };
  
  /** ICMP message reply */
  struct reply_t {
    message_t DATA;
    uint8_t TTL;
    uint8_t STATUS;
    uint8_t IP[4];
  };
  
  /** ICMP message type */
  enum {
    ICMP_ECHO_REPLY = 0,
    ICMP_DESTINATION_UNREACHABLE = 3,
    ICMP_SOURCE_QUENCHE = 4,
    ICMP_REDIRECT_MESSAGE = 5,
    ICMP_ECHO_REQUEST = 8,
    ICMP_ROUTER_ADVERTISEMENT = 9,
    ICMP_ROUTER_SOLICITATION = 10,
    ICMP_TIME_EXCEEDED = 11,
    ICMP_PARAMETER_PROBLEM = 12,
    ICMP_TIMESTAMP = 13,
    ICMP_TIMESTAMP_REPLY = 14,
    ICMP_INFORMATION_REPLY = 15,
    ICMP_INFORMATION_REQUEST = 16,
    ICMP_ADDRESS_MASK_REQUEST = 17,
    ICMP_ADDRESS_MASK_REPLY = 18,
    ICMP_TRACEROUT = 30    
  } __attribute__((packed));

  /** UDP socket */
  Socket* m_sock;
  
  uint8_t m_nextSeq;
  
  /**
   * Send a ICMP message to given IP address. Return zero if successful
   * otherwise negative error code.
   * @param[in] ip destination
   * @param[in] type ICMP message type option.
   * @param[in] code ICMP message code option (default 0)
   * @return zero if successful otherwise negative error code.
   */
  int send(uint8_t ip[4], uint8_t type, uint8_t code = 0);
  
  /**
   * Receive response of given type within the given time limit.
   * Return zero if successful otherwise negative error code.
   * @param[in] type ICMP message type option.
   * @param[in] code ICMP message code option (default 0)
   * @return zero if successful otherwise negative error code.
   */
  int recv(uint8_t ip[4], uint8_t type, uint8_t code = 0, uint16_t ms = 2000);
  
  Socket* socket(Socket::Protocol proto, uint16_t port = 0, uint8_t flag = 0);
};

#endif
