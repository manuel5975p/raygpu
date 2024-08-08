cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
FetchContent_Declare(
    dawn
    URL "${CMAKE_CURRENT_LIST_DIR}/dawn.tar.gz"
)
FetchContent_MakeAvailable(dawn)
#add_subdirectory(dawn)
add_executable(example "example.cpp" "InitWindow.cpp" "raygpu.h" "raygpu.cpp" "shapes.cpp")
target_compile_features(example PUBLIC cxx_std_20)
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