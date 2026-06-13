find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(UnitTests ${TEST_SOURCES})

target_link_libraries(UnitTests
        PRIVATE
        GLEngineLib
        GTest::gtest_main
)

gtest_discover_tests(UnitTests)