cmake_minimum_required(VERSION 3.10)

project(morph VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(helib REQUIRED)

add_executable(morph-cli src/main-cli.cpp src/client.cpp src/encryption.cpp src/network.cpp src/resp.cpp)
add_executable(morph-server src/main-server.cpp src/encryption.cpp src/network.cpp src/resp.cpp src/server.cpp src/store.cpp)

target_link_libraries(morph-cli helib)
target_link_libraries(morph-server helib)

install(TARGETS morph-cli RUNTIME DESTINATION bin)
install(TARGETS morph-server RUNTIME DESTINATION bin)
