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

#include <helib/helib.h>

namespace morph {

std::string ctxtToString(const helib::Ctxt& ctxt);

bool fileExists(const std::string& filename);

template <typename T1, typename T2>
using uniq_pair = std::pair<std::unique_ptr<T1>, std::unique_ptr<T2>>;

template <typename KEY>
uniq_pair<helib::Context, KEY> loadContextAndKey(const std::string& filename, bool secret) {
  if (!fileExists(filename)) {
    std::cerr << "No such file: " << filename << std::endl;
    std::cerr << "Use the " << (secret ? "-S" : "-P") << " option to specify a different path" << std::endl;
    std::cerr << "Or generate a key pair with `morph-cli keygen`" << std::endl;
    exit(1);
  }

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(1);
  }

  std::unique_ptr<helib::Context> contextp(helib::Context::readPtrFrom(file));
  std::unique_ptr<KEY> keyp;
  if constexpr (std::is_same_v<KEY, helib::SecKey>) {
    keyp = std::make_unique<helib::SecKey>(helib::SecKey::readFrom(file, *contextp, false));
  } else {
    keyp = std::make_unique<helib::PubKey>(helib::PubKey::readFrom(file, *contextp));
  }

  return {std::move(contextp), std::move(keyp)};
}

void generateKeys();

class Encryptor {
  public:
    Encryptor(const std::string& sk_path) {
      std::tie(contextp_, skp_) = loadContextAndKey<helib::SecKey>(sk_path, true);
    }
    std::string encrypt(const std::string& value);
    std::string decrypt(const std::string& value);

  private:
    std::vector<std::pair<helib::Ctxt, helib::Ctxt>> store_;
    std::shared_ptr<helib::Context> contextp_;
    std::unique_ptr<helib::SecKey> skp_;
};

}
