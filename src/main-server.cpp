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

#include "server.h"
#include "version.h"

struct Options {
  std::string bind = "127.0.0.1";
  int port = 6774;
  std::vector<std::string> args;
  bool help = false;
  bool version = false;
  std::string pk_path = "morph.pk";
  std::string err;
};

Options parseArgs(int argc, char *argv[]) {
  Options opts;

  int opt;
  while ((opt = getopt(argc, argv, ":p:b:P:hv")) != -1) {
    switch (opt) {
      case 'h':
        opts.help = true;
        break;
      case 'p':
        opts.port = atoi(optarg);
        break;
      case 'b':
        opts.bind = optarg;
        break;
      case 'P':
        opts.pk_path = optarg;
        break;
      case 'v':
        opts.version = true;
        break;
      case ':':
        opts.err = "Bad number of args: '-" + (std::string() + static_cast<char>(optopt)) + "'";
        break;
      case '?':
        opts.err = "Unrecognized option: '-" + (std::string() + static_cast<char>(optopt)) + "'";
        break;
    }
  }

  for (int i = optind; i < argc; i++) {
    opts.args.push_back(argv[i]);
  }

  if (!opts.args.empty()) {
    opts.help = true;
  }

  return opts;
}

void showUsage() {
  std::cerr
    << "Usage: morph-server [OPTIONS]" << std::endl
    << "  -p <port>          Port (default: 6774)" << std::endl
    << "  -b <address>       Bind address (default: 127.0.0.1)" << std::endl
    << "  -P <filename>      Path to public key (default: morph.pk)" << std::endl
    << "  -h                 Output this help and exit" << std::endl
    << "  -v                 Output version and exit" << std::endl;
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
    std::cout << "morph-server " << MORPH_VERSION << std::endl;
  } else {
    auto options = morph::ServerOptions();
    options.bind = opts.bind;
    options.port = opts.port;
    options.pk_path = opts.pk_path;
    auto server = morph::Server(options);
    server.start();
  }

  return 0;
}
