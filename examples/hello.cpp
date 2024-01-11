#include <morph/client.h>

int main() {
  auto morph = morph::Client();
  morph.flushall();
  morph.set("hello", "world");
  auto value = morph.get("hello");
  std::cout << value.value_or("(nil)") << std::endl;
}
