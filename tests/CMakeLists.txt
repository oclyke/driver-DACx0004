include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(dacx0004_tests
  mock_if.cpp
  test_null_guards.cpp
  test_bit_packing.cpp
  test_round_trip.cpp
  test_gpio_order.cpp
)

target_link_libraries(dacx0004_tests
  PRIVATE
  dacx0004_lib
  GTest::gtest_main
)

set_target_properties(dacx0004_tests PROPERTIES CXX_STANDARD 20)
target_include_directories(dacx0004_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(dacx0004_tests)
