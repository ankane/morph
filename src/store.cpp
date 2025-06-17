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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <helib/helib.h>

#include "store.h"

namespace morph {

helib::Ctxt Store::stringToCtxt(const std::string& str) {
  std::istringstream iss(str);
  return helib::Ctxt::readFrom(iss, *pkp_.get());
}

void Store::set(const std::string& key, const std::string& value) {
  auto encrypted_key = stringToCtxt(key);
  auto encrypted_value = stringToCtxt(value);
  store_.emplace_back(std::move(encrypted_key), std::move(encrypted_value));
}

std::string Store::get(const std::string& key) {
  if (store_.size() == 0) {
    return "";
  }

  auto encrypted_key = stringToCtxt(key);
  const helib::EncryptedArray& ea = contextp_->getEA();
  long p = contextp_->getP();

  std::vector<helib::Ctxt> mask;
  mask.reserve(store_.size());
  for (const auto& encrypted_pair : store_) {
    helib::Ctxt mask_entry = encrypted_pair.first;
    mask_entry -= encrypted_key;
    mask_entry.power(p - 1);
    mask_entry.negate();
    mask_entry.addConstant(NTL::ZZX(1));
    std::vector<helib::Ctxt> rotated_masks(ea.size(), mask_entry);
    for (int i = 1; i < rotated_masks.size(); i++) {
      ea.rotate(rotated_masks[i], i);
    }
    totalProduct(mask_entry, rotated_masks);
    mask_entry.multiplyBy(encrypted_pair.second);
    mask.push_back(mask_entry);
  }

  helib::Ctxt value = mask[0];
  for (int i = 1; i < mask.size(); i++) {
    value += mask[i];
  }
  return ctxtToString(value);
}

void Store::clear() {
  store_.clear();
}

std::vector<std::string> Store::keys() {
  std::vector<std::string> keys;
  for (const auto& v : store_) {
    keys.push_back(ctxtToString(v.first));
  }
  return keys;
}

int Store::size() {
  return store_.size();
}

} // namespace morph
