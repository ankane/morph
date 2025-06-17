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
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "network.h"
#include "resp.h"
#include "server.h"
#include "store.h"
#include "version.h"

namespace morph {

std::string wrongArgs(const std::string& cmd) {
  return respError("ERR wrong number of arguments for '" + cmd + "' command");
}

std::string processCommand(std::vector<std::string>& cmd, morph::Store& store) {
  std::string command = cmd[0];
  for (auto &c : command) {
    c = tolower(c);
  }

  int argc = cmd.size() - 1;

  if (command == "set") {
    if (argc != 2) {
      return wrongArgs("set");
    }
    store.set(cmd[1], cmd[2]);
    return respOk();
  } else if (command == "mset") {
    if (argc < 2 || argc % 2 != 0) {
      return wrongArgs("mset");
    }
    for (int i = 1; i < cmd.size(); i += 2) {
      store.set(cmd[i], cmd[i + 1]);
    }
    return respOk();
  } else if (command == "get") {
    if (argc != 1) {
      return wrongArgs("get");
    }
    return respBulkString(store.get(cmd[1]));
  } else if (command == "mget") {
    if (argc < 1) {
      return wrongArgs("mget");
    }
    std::vector<std::string> vec;
    for (int i = 1; i < cmd.size(); i++) {
      vec.push_back(store.get(cmd[i]));
    }
    return respArray(vec);
  } else if (command == "flushall") {
    if (argc != 0) {
      return wrongArgs("flushall");
    }
    store.clear();
    return respOk();
  } else if (command == "dbsize") {
    if (argc != 0) {
      return wrongArgs("dbsize");
    }
    // can't get unique keys
    return respInteger(store.size());
  } else if (command == "keys") {
    if (argc != 1) {
      return wrongArgs("keys");
    }
    if (cmd[1] != "*") {
      return respError("ERR only '*' supported");
    }
    return respArray(store.keys());
  } else if (command == "info") {
    // sections not supported yet
    if (argc != 0) {
      return wrongArgs("info");
    }
    std::string str = "# Server\r\nmorph_version:" + std::string(MORPH_VERSION) + "\r\n";
    return respBulkString(str);
  } else {
    return respError("ERR unknown command '" + command + "'");
  }
}

void Server::handleError(const std::string& section, const std::string& message) {
  std::cerr
    << "Could not create server TCP listening socket "
    << options_.bind << ":" << options_.port << ": "
    << section << ": " << message << std::endl;
  exit(1);
}

void Server::start() {
  auto store = morph::Store(options_.pk_path);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    handleError("socket", std::strerror(errno));
  }

  int v = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int)) == -1) {
    handleError("setsockopt", std::strerror(errno));
  }

  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(options_.bind.c_str());
  sockaddr.sin_port = htons(options_.port);
  if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
    handleError("bind", std::strerror(errno));
  }

  int max_connections = 10;
  if (listen(sockfd, max_connections) < 0) {
    handleError("listen", std::strerror(errno));
  }

  std::cerr << "Ready to accept connections" << std::endl;

  char buffer[1048576] = {0};

  while (1) {
    auto addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (connection < 0) {
      handleError("accept", std::strerror(errno));
    }

    auto bytesRead = connRead(connection, buffer, 1048576);
    if (bytesRead > 0) {
      auto arr = readArray(buffer);
      std::string response = processCommand(arr, store);
      send(connection, response.c_str(), response.size(), 0);
    } else if (bytesRead == -2) {
      std::string response = respError("Request too large for current implementation");
      send(connection, response.c_str(), response.size(), 0);
    } else {
      std::cerr << "No bytes read" << std::endl;
    }
  }

  close(sockfd);
}

} // namespace morph
