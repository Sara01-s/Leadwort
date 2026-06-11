find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(unit_tests main_test.cpp)

target_link_libraries(unit_tests
        PRIVATE
        GLEngineLib
        GTest::gtest_main
)

gtest_discover_tests(unit_tests)