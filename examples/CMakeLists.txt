cmake_minimum_required(VERSION 3.28)
project(randomshake-example LANGUAGES CXX)

find_package(randomshake QUIET)

if(NOT randomshake_FOUND)
  message(STATUS
    "randomshake not found via find_package, using FetchContent...")
  include(FetchContent)
  FetchContent_Declare(
    randomshake
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
  )
  FetchContent_MakeAvailable(randomshake)
endif()

add_executable(csprng_generate_byte_seq csprng_generate_byte_seq.cpp)
target_link_libraries(csprng_generate_byte_seq PRIVATE randomshake)
target_compile_features(csprng_generate_byte_seq PRIVATE cxx_std_20)
