# Morph

:fire: An encrypted, in-memory, key-value store

Uses homomorphic encryption, so the server can’t read data or queries. Powered by [HElib](https://github.com/homenc/HElib) and follows the [Redis protocol](https://redis.io/topics/protocol).

Learn [how it works](https://github.com/homenc/HElib/tree/master/examples/BGV_country_db_lookup#country-lookup-example).

*Designed for research and education, and should not be considered secure. For a more practical approach to key-value store encryption, check out [Cloak](https://github.com/ankane/cloak).*

[![Build Status](https://github.com/ankane/morph/workflows/build/badge.svg?branch=master)](https://github.com/ankane/morph/actions)

## Installation

On Mac, run:

```sh
brew install ankane/brew/morph
```

On other platforms, [build from source](#building-from-source).

## Getting Started

Generate a key pair

```sh
morph-cli keygen
```

Start the server (in another window)

```sh
morph-server
```

Set a key

```sh
morph-cli set hello world
```

**Note:** Each key should only be set once, or the value will not be recoverable

Get a key

```sh
morph-cli get hello
```

Set multiple keys

```sh
morph-cli mset key1 hello key2 world
```

Get multiple keys

```sh
morph-cli mget key1 key2
```

Delete all keys

```sh
morph-cli flushall
```

Get the number of keys

```sh
morph-cli dbsize
```

List keys

```sh
morph-cli keys "*"
```

Get info

```sh
morph-cli info
```

## Time Complexity

- set - O(1)
- get - O(N) where N is the number of keys in the store
- mset - O(N) where N is the number of keys to set
- mget - O(N*M) where N is the number of keys to get and M is the number of keys in the store
- keys - O(N) where N is the number of keys in the store

## Clients

- [C++](#c)
- [Ruby](https://github.com/ankane/morph-ruby)

### C++

Create `hello.cpp`:

```cpp
#include <morph/client.h>

int main() {
  auto morph = morph::Client();
  morph.flushall();
  morph.set("hello", "world");
  auto value = morph.get("hello");
  std::cout << value.value_or("(nil)") << std::endl;
}
```

Compile:

```sh
g++ -std=c++17 hello.cpp -lmorph -lpthread -lhelib -lntl -o hello
```

And run:

```sh
./hello
```

## Building from Source

First, install HElib.

On Ubuntu 22.04, use:

```sh
sudo apt-get install libntl-dev
git clone --branch v2.3.0 https://github.com/homenc/HElib.git
cd HElib && mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig
```

On Mac, use:

```sh
brew install helib
```

Then run:

```sh
git clone https://github.com/ankane/morph.git
cd morph && mkdir build && cd build
cmake ..
make
make install # optional, may need sudo
```

## Credits

Thanks to IBM for HElib and Redis for the protocol/commands. Based on [this example](https://github.com/homenc/HElib/tree/master/examples/BGV_country_db_lookup).

## History

View the [changelog](https://github.com/ankane/morph/blob/master/CHANGELOG.md)

## Contributing

Everyone is encouraged to help improve this project. Here are a few ways you can help:

- [Report bugs](https://github.com/ankane/morph/issues)
- Fix bugs and [submit pull requests](https://github.com/ankane/morph/pulls)
- Write, clarify, or fix documentation
- Suggest or add new features
