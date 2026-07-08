find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(LeadwortEngineTests ${TEST_SOURCES})

target_include_directories(LeadwortEngineTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

target_link_libraries(LeadwortEngineTests
        PRIVATE
            LeadwortEngineLib
            GTest::gtest_main
            GTest::gmock
)

gtest_discover_tests(LeadwortEngineTests)