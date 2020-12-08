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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "resp.h"

namespace morph {

std::string respOk() {
  return "+OK\r\n";
}

std::string respError(const std::string& value) {
  return "-" + value + "\r\n";
}

std::string respInteger(int value) {
  return ":" + std::to_string(value) + "\r\n";
}

std::string respBulkString(const std::string& value) {
  // RESP treats empty string separate from null
  // but we use empty string to represent null
  if (value.empty()) {
    return "$-1\r\n";
  } else {
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
  }
}

std::string respArray(const std::vector<std::string>& value) {
  std::ostringstream oss;
  oss << "*" << value.size() << "\r\n";
  for (auto &v : value) {
    oss << respBulkString(v);
  }
  return oss.str();
}

std::string readBulkString(const char*& p) {
  if (*p != '$') {
    // TODO throw error
    std::cerr << "Bad bulk string" << std::endl;
    return "";
  }

  int len = 0;

  p++;
  while(*p != '\r') {
      len = (len*10)+(*p - '0');
      p++;
  }
  p += 2;

  std::string str(p, len);
  p += (len + 2);
  return str;
}

std::vector<std::string> readArray(const char* buffer) {
  std::vector<std::string> vec;

  if (buffer[0] != '*') {
    // TODO throw error
    std::cerr << "Bad array" << std::endl;
    return vec;
  }

  const char *p = buffer;

  int len = 0;

  p++;
  while(*p != '\r') {
    len = (len*10)+(*p - '0');
    p++;
  }
  p += 2;

  vec.reserve(len);
  for (int i = 0; i < len; i++) {
    vec.push_back(readBulkString(p));
  }
  return vec;
}

Result readResult(const std::string& str) {
  const char *buffer = str.c_str();

  Result res;
  switch(str[0]) {
    case '+':
      res.type = RESP_SIMPLE_STRING;
      res.value_str = str.substr(1, str.size() - 2);
      break;
    case '-':
      res.type = RESP_ERROR;
      res.value_str = str.substr(1, str.size() - 2);
      break;
    case ':':
      res.type = RESP_INTEGER;
      res.value_int = std::atoi(str.substr(1, str.size() - 2).c_str());
      break;
    case '$':
      res.type = RESP_BULK_STRING;
      // TODO handle null in readBulkString
      res.value_str = (str == "$-1\r\n") ? "" : readBulkString(buffer);
      break;
    case '*':
      res.type = RESP_ARRAY;
      res.value_arr = readArray(buffer);
      break;
    default:
      res.type = RESP_UNKNOWN;
  }
  return res;
}

}
