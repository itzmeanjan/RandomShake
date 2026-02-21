cmake_minimum_required(VERSION 3.28)

project(randomshake VERSION 0.1.0 LANGUAGES CXX)

# --- Options ---
option(RANDOMSHAKE_BUILD_TESTS "Build tests" OFF)
option(RANDOMSHAKE_BUILD_EXAMPLES "Build examples" OFF)
option(RANDOMSHAKE_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(RANDOMSHAKE_FETCH_DEPS "Fetch missing dependencies (GTest, Benchmark)" OFF)

# --- Top-level-only settings (skipped when consumed via FetchContent/add_subdirectory) ---
if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  option(RANDOMSHAKE_ENABLE_LTO "Enable Interprocedural Optimization (LTO)" ON)
  option(RANDOMSHAKE_NATIVE_OPT "Enable -march=native (not suitable for cross-compilation)" OFF)
  option(RANDOMSHAKE_ASAN "Enable AddressSanitizer" OFF)
  option(RANDOMSHAKE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

  # --- Tools ---
  find_program(CLANG_TIDY_EXE clang-tidy)
  find_program(CLANG_FORMAT_EXE clang-format)

  if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE TIDY_SOURCES "examples/*.cpp")

    add_custom_target(tidy
      COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy --header-filter=${CMAKE_CURRENT_SOURCE_DIR}/include/randomshake/.* ${TIDY_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running clang-tidy"
    )
  endif()

  if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE FORMAT_SOURCES
      "include/**/*.hpp" "examples/*.cpp"
      "benches/*.hpp" "benches/*.cpp"
      "tests/*.hpp" "tests/*.cpp")

    add_custom_target(format
      COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${FORMAT_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running clang-format"
    )
  endif()

  # --- Standard settings ---
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  # --- Compiler Warnings ---
  if(MSVC)
    set(RANDOMSHAKE_WARNING_FLAGS /W4)
  else()
    set(RANDOMSHAKE_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wcast-qual -Wold-style-cast -Wundef -Werror)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      list(APPEND RANDOMSHAKE_WARNING_FLAGS -Wno-pass-failed)
    endif()
  endif()

  # --- Sanitizers ---
  if(RANDOMSHAKE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)
    add_link_options(-fsanitize=address)
  endif()

  if(RANDOMSHAKE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-sanitize-recover=undefined)
    add_link_options(-fsanitize=undefined)
  endif()

  # --- Native Optimization ---
  if(RANDOMSHAKE_NATIVE_OPT)
    add_compile_options(-march=native)
    message(STATUS "Enabled -march=native (machine-specific optimization)")
  endif()

  # --- LTO ---
  if(RANDOMSHAKE_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
      message(WARNING "LTO is not supported: ${output}")
    endif()
  endif()
endif()

# --- Dependency: sha3 (via FetchContent) ---
include(FetchContent)
FetchContent_Declare(
  sha3
  GIT_REPOSITORY https://github.com/itzmeanjan/sha3.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sha3)

# --- Library (header-only → INTERFACE) ---
add_library(randomshake INTERFACE)
target_include_directories(randomshake INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(randomshake INTERFACE sha3)
target_compile_features(randomshake INTERFACE cxx_std_20)

# --- Tests ---
if(RANDOMSHAKE_BUILD_TESTS)
  enable_testing()
  if(RANDOMSHAKE_FETCH_DEPS)
    message(STATUS "Fetching GTest...")
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  else()
    find_package(GTest REQUIRED)
  endif()

  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

  add_executable(randomshake_tests ${TEST_SOURCES})
  target_link_libraries(randomshake_tests PRIVATE randomshake GTest::gtest_main)
  target_include_directories(randomshake_tests PRIVATE tests)
  target_compile_options(randomshake_tests PRIVATE ${RANDOMSHAKE_WARNING_FLAGS})

  include(GoogleTest)
  gtest_discover_tests(randomshake_tests)
endif()

# --- Benchmarks ---
if(RANDOMSHAKE_BUILD_BENCHMARKS)
  if(RANDOMSHAKE_FETCH_DEPS)
    message(STATUS "Fetching Google Benchmark...")
    include(FetchContent)
    FetchContent_Declare(
      benchmark
      URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
  else()
    find_package(benchmark REQUIRED)
  endif()

  file(GLOB BENCHMARK_SOURCES CONFIGURE_DEPENDS "benches/*.cpp")
  find_library(LIBPFM pfm)

  add_executable(randomshake_benchmarks ${BENCHMARK_SOURCES})
  target_link_libraries(randomshake_benchmarks PRIVATE randomshake benchmark::benchmark_main)

  if(LIBPFM)
    target_link_libraries(randomshake_benchmarks PRIVATE ${LIBPFM})
    message(STATUS "Found libpfm: ${LIBPFM} - linking it to benchmarks")
  endif()

  target_include_directories(randomshake_benchmarks PRIVATE benches)
  target_compile_options(randomshake_benchmarks PRIVATE ${RANDOMSHAKE_WARNING_FLAGS})
endif()

# --- Examples ---
if(RANDOMSHAKE_BUILD_EXAMPLES)
  file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "examples/*.cpp")
  foreach(ex_src ${EXAMPLE_SOURCES})
    get_filename_component(ex_name ${ex_src} NAME_WE)
    set(ex_target "${ex_name}_example")
    add_executable(${ex_target} ${ex_src})
    target_link_libraries(${ex_target} PRIVATE randomshake)
    target_compile_options(${ex_target} PRIVATE ${RANDOMSHAKE_WARNING_FLAGS})
  endforeach()
endif()

# --- Install ---
include(GNUInstallDirs)
install(TARGETS randomshake EXPORT randomshake-config INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT randomshake-config NAMESPACE randomshake:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/randomshake)
