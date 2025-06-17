/*
 * Copyright (C) 2020 Andrew Kane
 *
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "network.h"

namespace morph {

int connSend(char const *hostname, int port, const std::string& oss) {
  int sd = -1, err;
  struct addrinfo hints = {}, *addrs;
  char port_str[16] = {};

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  snprintf(port_str, 15, "%d", port);

  err = getaddrinfo(hostname, port_str, &hints, &addrs);
  if (err != 0) {
    fprintf(stderr, "Could not connect to Morph at %s:%d: %s\n", hostname, port, std::strerror(err));
    exit(1);
  }

  for (struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next) {
    sd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sd == -1) {
      err = errno;
      break;
    }

    if (connect(sd, addr->ai_addr, addr->ai_addrlen) == 0) {
      break;
    }

    err = errno;

    close(sd);
    sd = -1;
  }

  freeaddrinfo(addrs);

  if (sd == -1) {
    fprintf(stderr, "Could not connect to Morph at %s:%d: %s\n", hostname, port, std::strerror(err));
    exit(1);
  }

  send(sd, oss.c_str(), oss.size(), 0);
  return sd;
}

// TODO remove this hack and read data properly
int connRead(int connection, char* buffer, int buffer_size) {
  auto bytesRead = read(connection, buffer, buffer_size);
  auto result = bytesRead;
  while (result > 0 && bytesRead < buffer_size && buffer[bytesRead - 2] != '\r' && buffer[bytesRead - 1] != '\n') {
    result = read(connection, buffer + bytesRead, buffer_size - bytesRead);
    if (result < 1) {
      return -1;
    }
    bytesRead += result;
  }

  if (buffer[bytesRead - 2] != '\r' && buffer[bytesRead - 1] != '\n') {
    return -2;
  }

  return bytesRead;
}

} // namespace morph
