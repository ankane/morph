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

void readKeyBinary(std::istream& filename, helib::PubKey& pk);
void readKeyBinary(std::istream& filename, helib::SecKey& sk);

template <typename T1, typename T2>
using uniq_pair = std::pair<std::unique_ptr<T1>, std::unique_ptr<T2>>;

template <typename KEY>
uniq_pair<helib::Context, KEY> loadContextAndKey(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(1);
  }

  unsigned long m, p, r;
  std::vector<long> gens, ords;

  helib::readContextBaseBinary(file, m, p, r, gens, ords);
  std::unique_ptr<helib::Context> contextp = std::make_unique<helib::Context>(m, p, r, gens, ords);
  helib::readContextBinary(file, *contextp);

  std::unique_ptr<KEY> keyp = std::make_unique<KEY>(*contextp);
  readKeyBinary(file, *keyp);

  return {std::move(contextp), std::move(keyp)};
}

void generateKeys();

class Encryptor {
  public:
    Encryptor(const std::string& sk_path) {
      std::tie(contextp_, skp_) = loadContextAndKey<helib::SecKey>(sk_path);
    }
    std::string encrypt(const std::string& value);
    std::string decrypt(const std::string& value);

  private:
    std::vector<std::pair<helib::Ctxt, helib::Ctxt>> store_;
    std::shared_ptr<helib::Context> contextp_;
    std::unique_ptr<helib::SecKey> skp_;
};

}
