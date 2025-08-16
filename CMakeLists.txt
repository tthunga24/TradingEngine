cmake_minimum_required(VERSION 3.15)
project(TradingEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependency Management ---
include(FetchContent)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.14.1)
FetchContent_Declare(cppzmq GIT_REPOSITORY https://github.com/zeromq/cppzmq.git GIT_TAG v4.10.0)

set(CPPZMQ_BUILD_TESTS OFF CACHE INTERNAL "")
set(CPPZMQ_BUILD_EXAMPLES OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(json spdlog cppzmq)

find_package(Threads REQUIRED)

find_library(BID_LIBRARY_PATH NAMES bid)
if(NOT BID_LIBRARY_PATH)
    message(FATAL_ERROR "Intel DFP library (libbid) not found. Please ensure it is installed correctly (e.g., yay -S intel-dfp).")
endif()
message(STATUS "Found Intel DFP library at: ${BID_LIBRARY_PATH}")


# --- Source File Management ---
file(GLOB ENGINE_SOURCES "src/*.cpp")

file(GLOB IBKR_API_SOURCES "vendor/ibkr/*.cpp")
#file(GLOB IBKR_DECIMAL_SOURCES "vendor/ibkr/bid64_string.c")

# --- Main Executable Target ---
add_executable(engine
  ${ENGINE_SOURCES}
  ${IBKR_API_SOURCES}
  # ${IBKR_DECIMAL_SOURCES}
)

target_include_directories(engine PUBLIC
  include
  vendor/ibkr
)

target_link_libraries(engine PRIVATE
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  cppzmq
  Threads::Threads
  ${BID_LIBRARY_PATH}
  m
  ${BID_LIBRARY_PATH}
)


# --- Unit Testing Setup ---
enable_testing()
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

add_executable(order_manager_test
  tests/test_ordermanager.cpp
  src/OrderManager.cpp
)

target_link_libraries(order_manager_test PRIVATE
  GTest::gtest_main
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  cppzmq
)

target_include_directories(order_manager_test PUBLIC include)
include(GoogleTest)
gtest_discover_tests(order_manager_test)
