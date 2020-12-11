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

#include <fstream>
#include <iostream>

#include "client.h"
#include "encryption.h"
#include "network.h"
#include "resp.h"

namespace morph {

void Client::keygen() {
  generateKeys();
}

// TODO use OptionalString object
std::string decrypt(morph::Encryptor& encryptor, const std::string& str) {
  auto decrypted = encryptor.decrypt(str);
  if (decrypted.empty()) {
    return decrypted;
  }
  if (decrypted[0] != '+') {
    return "(set multiple times)";
  }
  // remove trailing NUL bytes
  // TODO set length in value
  return std::string(decrypted.substr(1).c_str());
}

Result Client::execute(std::vector<std::string>& args) {
  // encrypt
  auto encryptor = morph::Encryptor(options_.sk_path);
  std::vector<std::string> arr;
  arr.reserve(args.size());
  for (int i = 0; i < args.size(); i++) {
    if (i == 0 || (args[0] == "keys" && args[i] == "*")) {
      arr.push_back(args[i]);
    } else {
      // TODO use hash of data instead?
      arr.push_back(encryptor.encrypt("+" + args[i]));
    }
  }

  // serialize
  auto serialized = respArray(arr);

  // send and receive
  auto sock = connSend(options_.hostname.c_str(), options_.port, serialized);
  char buffer[1048576] = {0};
  auto bytesRead = connRead(sock, buffer, 1048576);
  if (bytesRead <= 0) {
    std::cerr << "No bytes read" << std::endl;
    exit(1);
  }

  // deserialize
  auto res = readResult(std::string(buffer, bytesRead));

  // decrypt
  if (res.type == RESP_BULK_STRING && args[0] != "info") {
    res.value_str = decrypt(encryptor, res.value_str);
  } else if (res.type == RESP_ARRAY) {
    for (int i = 0; i < res.value_arr.size(); i++) {
      res.value_arr[i] = decrypt(encryptor, res.value_arr[i]);
    }
  }

  return res;
}

bool Client::set(const std::string& key, const std::string& value) {
  std::vector<std::string> args {"set", key, value};
  auto res = execute(args);
  return res.value_str == "OK";
}

std::string Client::get(const std::string& key) {
  std::vector<std::string> args {"get", key};
  auto res = execute(args);
  return res.value_str;
}

void Client::flushall() {
  std::vector<std::string> args {"flushall"};
  execute(args);
}

int Client::dbsize() {
  std::vector<std::string> args {"dbsize"};
  auto res = execute(args);
  return res.value_int;
}

std::vector<std::string> Client::keys(const std::string& pattern) {
  std::vector<std::string> args {"keys", pattern};
  auto res = execute(args);
  return res.value_arr;
}

std::string Client::info() {
  std::vector<std::string> args {"info"};
  auto res = execute(args);
  return res.value_str;
}

}
