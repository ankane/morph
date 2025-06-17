/*
 * Copyright (C) 2020 IBM Corp.
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

#include <memory>
#include <string>
#include <vector>

#include <helib/helib.h>

#include "encryption.h"

namespace morph {

class Store {
  public:
    Store(const std::string& pk_path) {
      std::tie(contextp_, pkp_) = loadContextAndKey<helib::PubKey>(pk_path, false);
    }
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    void clear();
    std::vector<std::string> keys();
    int size();

  private:
    std::vector<std::pair<helib::Ctxt, helib::Ctxt>> store_;
    std::shared_ptr<helib::Context> contextp_;
    std::unique_ptr<helib::PubKey> pkp_;

    helib::Ctxt stringToCtxt(const std::string& str);
};

} // namespace morph
