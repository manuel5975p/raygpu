cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
#set(CMAKE_CXX_FLAGSE "${CMAKE_CXX_FLAGS} -flto")
FetchContent_Declare(
    dawn
    URL "${CMAKE_CURRENT_LIST_DIR}/dawn"
)
FetchContent_MakeAvailable(dawn)

add_executable(example "example.cpp" "InitWindow.cpp" "raygpu.h" "vertex_buffer.hpp")
target_compile_features(ass PUBLIC cxx_std_20)
include_directories("${CMAKE_CURRENT_LIST_DIR}/dawn/include")
target_link_libraries(example PUBLIC
    webgpu_dawn
    webgpu_cpp
    dawn_internal_config
    dawn::dawn_glfw
    glfw

    #dawn::dawn_native
    #dawn::dawn_common
    #dawn::dawn_system_utils
    #dawn::dawn_wgpu_utils
    #dawn::dawn_test_utils
    #dawn::dawn_proc
)