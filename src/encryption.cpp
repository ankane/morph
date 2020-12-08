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

#include <sys/stat.h>

#include "encryption.h"

namespace morph {

std::string ctxtToString(const helib::Ctxt& ctxt) {
  std::ostringstream oss;
  ctxt.write(oss);
  return oss.str();
}

void readKeyBinary(std::istream& filename, helib::PubKey& pk) {
  helib::readPubKeyBinary(filename, pk);
}

void readKeyBinary(std::istream& filename, helib::SecKey& sk) {
  helib::readSecKeyBinary(filename, sk);
}

bool fileExists(const std::string& filename)
{
  struct stat buf;
  return stat(filename.c_str(), &buf) == 0;
}

std::ofstream createFile(const std::string& filename) {
  if (fileExists(filename)) {
    std::cerr << "File already exists: " << filename << std::endl;
    exit(1);
  }

  std::ofstream file;
  file.open(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(1);
  }
  return file;
}

void generateKeys() {
  unsigned long p = 131;
  unsigned long m = 130;
  unsigned long r = 1;
  unsigned long bits = 1000;
  unsigned long c = 2;
  unsigned long nthreads = 1;

  helib::Context context(m, p, r);
  helib::buildModChain(context, bits, c);
  helib::SecKey secret_key = helib::SecKey(context);
  secret_key.GenSecKey();
  helib::addSome1DMatrices(secret_key);
  const helib::PubKey& public_key = secret_key;

  auto sk_file = createFile("morph.sk");
  helib::writeContextBaseBinary(sk_file, context);
  helib::writeContextBinary(sk_file, context);
  helib::writeSecKeyBinary(sk_file, secret_key);
  sk_file.close();

  auto pk_file = createFile("morph.pk");
  helib::writeContextBaseBinary(pk_file, context);
  helib::writeContextBinary(pk_file, context);
  helib::writePubKeyBinary(pk_file, public_key);
  pk_file.close();
}

std::string Encryptor::encrypt(const std::string& value) {
  helib::PubKey public_key = *skp_;

  helib::Ptxt<helib::BGV> plaintext_value(public_key.getContext());
  for (long i = 0; i < value.size(); ++i) {
    plaintext_value.at(i) = value[i];
  }

  helib::Ctxt encrypted_value(public_key);
  public_key.Encrypt(encrypted_value, plaintext_value);

  return ctxtToString(encrypted_value);
}

std::string Encryptor::decrypt(const std::string& str) {
  if (str.empty()) {
    return "";
  }

  std::istringstream iss(str);
  helib::Ctxt encrypted_result(*skp_);
  encrypted_result.read(iss);

  helib::Ptxt<helib::BGV> plaintext_result(skp_->getContext());
  skp_->Decrypt(plaintext_result, encrypted_result);

  std::string string_result;
  for (long i = 0; i < plaintext_result.size(); ++i) {
    string_result.push_back(static_cast<long>(plaintext_result[i]));
  }

  if (string_result.at(0) == 0x00) {
    return "";
  } else {
    return string_result;
  }
}

}
