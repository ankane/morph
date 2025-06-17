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
#include <string>
#include <unistd.h>
#include <vector>

#include "client.h"
#include "version.h"

struct Options {
  std::string hostname = "127.0.0.1";
  int port = 6774;
  std::vector<std::string> args;
  bool help = false;
  bool version = false;
  std::string sk_path = "morph.sk";
  std::string err;
};

Options parseArgs(int argc, char *argv[]) {
  Options opts;

  int opt;
  while ((opt = getopt(argc, argv, ":h:p:S:v")) != -1) {
    switch (opt) {
      case 'h':
        opts.hostname = optarg;
        break;
      case 'p':
        opts.port = std::atoi(optarg);
        break;
      case 'S':
        opts.sk_path = optarg;
        break;
      case 'v':
        opts.version = true;
        break;
      case ':':
        if (optopt != 'h') {
          opts.err = "Bad number of args: '-" + (std::string() + static_cast<char>(optopt)) + "'";
        }
        break;
      case '?':
        opts.err = "Unrecognized option: '-" + (std::string() + static_cast<char>(optopt)) + "'";
        break;
    }
  }

  if (optind >= argc) {
    opts.help = true;
  }

  for (int i = optind; i < argc; i++) {
    opts.args.push_back(argv[i]);
  }

  return opts;
}

void showUsage() {
  std::cerr
    << "Usage: morph-cli [OPTIONS] [cmd [arg [arg ...]]]" << std::endl
    << "  -h <hostname>      Server hostname (default: 127.0.0.1)" << std::endl
    << "  -p <port>          Server port (default: 6774)" << std::endl
    << "  -S <filename>      Path to secret key (default: morph.sk)" << std::endl
    << "  -h                 Output this help and exit" << std::endl
    << "  -v                 Output version and exit" << std::endl << std::endl
    << "Examples:" << std::endl
    << "  morph-cli keygen" << std::endl
    << "  morph-cli set hello world" << std::endl
    << "  morph-cli get hello" << std::endl;
}

// TODO accept std::optional<std::string>
std::string inspectString(const std::string& value) {
  if (value.empty()) {
    return "(nil)";
  } else if (value == "(set multiple times)") {
    return value;
  } else {
    return '"' + value + '"';
  }
}

void printArray(std::vector<std::string> arr) {
  if (arr.empty()) {
    std::cout << "(empty list or set)" << std::endl;
  } else {
    for (int i = 0; i < arr.size(); i++) {
      std::cout << (i + 1) << ") " << inspectString(arr[i]) << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  auto opts = parseArgs(argc, argv);

  if (!opts.err.empty()) {
    std::cerr << opts.err << std::endl;
    return 1;
  } else if (opts.help) {
    showUsage();
    return 1;
  } else if (opts.version) {
    std::cout << "morph-cli " << MORPH_VERSION << std::endl;
  } else if (opts.args[0] == "keygen") {
    if (opts.args.size() > 1) {
      std::cerr << "Too many arguments" << std::endl;
      return 1;
    }
    auto morph = morph::Client();
    morph.keygen();
    std::cerr << "Generated morph.sk (secret key) and morph.pk (public key)" << std::endl;
  } else {
    auto options = morph::ClientOptions();
    options.hostname = opts.hostname;
    options.port = opts.port;
    options.sk_path = opts.sk_path;
    auto morph = morph::Client(options);
    auto res = morph.execute(opts.args);

    switch(res.type) {
      case morph::RESP_SIMPLE_STRING:
        std::cout << res.value_str << std::endl;
        break;
      case morph::RESP_ERROR:
        std::cout << "(error) " << res.value_str << std::endl;
        return 1;
      case morph::RESP_INTEGER:
        std::cout << std::to_string(res.value_int) << std::endl;
        break;
      case morph::RESP_BULK_STRING:
        if (opts.args[0] == "info") {
          std::cout << res.value_str;
        } else {
          std::cout << inspectString(res.value_str) << std::endl;
        }
        break;
      case morph::RESP_ARRAY:
        printArray(res.value_arr);
        break;
      case morph::RESP_UNKNOWN:
        std::cout << "(error) Unknown response" << std::endl;
        return -1;
    }
  }

  return 0;
}
