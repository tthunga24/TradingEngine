cmake_minimum_required(VERSION 3.15)

project(TradingEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependency management

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.14.1
)

FetchContent_Declare(
  cppzmq
  GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
  GIT_TAG        v4.10.0
)

set(CPPZMQ_BUILD_TESTS OFF CACHE INTERNAL "")
set(CPPZMQ_BUILD_EXAMPLES OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(json spdlog cppzmq)
# End of dependency management


# add new .cpp files in src here
add_executable(engine
  src/main.cpp
  src/ConfigHandler.cpp
  src/OrderManager.cpp
  src/EngineCore.cpp
  src/MockMarketDataHandler.cpp
  src/ScriptingInterface.cpp
)

target_include_directories(engine PUBLIC include)

target_link_libraries(engine PRIVATE
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  cppzmq
)


# Unit test setups 
enable_testing()
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
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
