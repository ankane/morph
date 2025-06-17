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

#include <string>
#include <vector>

namespace morph {

enum RESP_TYPE { RESP_UNKNOWN, RESP_SIMPLE_STRING, RESP_ERROR, RESP_INTEGER, RESP_BULK_STRING, RESP_ARRAY };

struct Result {
  RESP_TYPE type;
  std::string value_str;
  int value_int;
  std::vector<std::string> value_arr;
};

std::string respOk();
std::string respError(const std::string& value);
std::string respInteger(int value);
std::string respBulkString(const std::string& value);
std::string respArray(const std::vector<std::string>& value);

std::string readBulkString(const char*& p);
std::vector<std::string> readArray(const char* buffer);
Result readResult(const std::string& str);

} // namespace morph
