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

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "resp.h"

namespace morph {

struct ClientOptions {
  std::string hostname = "127.0.0.1";
  int port = 6774;
  std::string sk_path = "morph.sk";
};

class Client {
  public:
    Client() {};
    Client(ClientOptions& options) {
      options_ = options;
    };

    void keygen();

    // TODO support dynamic args
    // TODO better return type
    Result execute(std::vector<std::string>& cmd);

    bool set(const std::string& key, const std::string& value);
    // TODO return optional string
    std::string get(const std::string& key);

    // TODO add mset and mget

    void flushall();
    int dbsize();
    std::vector<std::string> keys(const std::string& pattern = "*");
    std::string info();

  private:
    ClientOptions options_;

};

}
