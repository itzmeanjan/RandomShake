# RandomSHAKE Example

A standalone example project demonstrating how to use the [RandomSHAKE](https://github.com/itzmeanjan/RandomShake) library.

## Quick Start

```bash
cd examples

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run
./build/csprng_generate_byte_seq
```

## Using `randomshake` in Your Own Project

The simplest way to integrate `randomshake` into your CMake project is via `FetchContent`:

```cmake
cmake_minimum_required(VERSION 3.28)
project(my_app LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  randomshake
  GIT_REPOSITORY https://github.com/itzmeanjan/RandomShake.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(randomshake)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE randomshake)
target_compile_features(my_app PRIVATE cxx_std_20)
```

Alternatively, install `randomshake` system-wide and use `find_package`:

```bash
# From the RandomShake root directory
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build
```

```cmake
find_package(randomshake REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE randomshake)
```
