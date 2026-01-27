if(NOT TARGET gtest)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    FetchContent_MakeAvailable(googletest)
endif()
add_executable(rg_tests "rg_tests.cpp" "rg_test_main.cpp" "test_commandbuffer.cpp")
target_link_libraries(rg_tests PRIVATE raygpu GTest::gtest)
gtest_discover_tests(rg_tests 
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    TEST_PREFIX "RayGPU."
)
