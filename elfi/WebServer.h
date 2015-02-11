/*
 * @flie WebServer.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2015,      Alexander Sehlström
 * Copyright (C) 2013-2015, Mikael Patel (original web server implementation of
 *                                        HTTP::Server)
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
 */
 
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "Cosa/INET/HTTP.hh"
#include "Cosa/IOStream.hh"

// HTML end of line
#define CRLF "\r\n"

/**
 * Subclass of HTTP::Server, a server request handler class. Implements project
 * specific on_request() function to produce response to HTTP requests.
 *
 * @section References
 * 1. CosaPinWebServer.ino example file.
 * https://github.com/mikaelpatel/Cosa/tree/master/examples/Ethernet/CosaPinWebServer
 */
class WebServer : public HTTP::Server
{
  public:
    /**
     * Default constructor.
     */
    WebServer() {}

    /**
     * Override of the HTTP::Server:on_request() member function. Displays
     * the ElFi web application controll and communicats with the rest of the
     * controll system.
     * @param[in] page iostream for response.
     * @param[in] method http request method string.
     * @param[in] path resource path string.
     * @param[in] query possible query string.
     */
    virtual void on_request(IOStream& page, char* method, char* path, char* query);
};

#endif
