cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
#FetchContent_Declare(
#    dawn
#    URL "${CMAKE_CURRENT_LIST_DIR}/dawn.tar.gz"
#)
#FetchContent_MakeAvailable(dawn)
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dawn)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# Define the download function
function(download_file_if_not_exists url filename)
    set(DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/dl")
    set(DOWNLOAD_FILE "${DOWNLOAD_DIR}/${filename}")

    # Create download directory if it doesn't exist
    file(MAKE_DIRECTORY ${DOWNLOAD_DIR})

    # Check if the file exists, download if not
    if(NOT EXISTS ${DOWNLOAD_FILE})
        message(STATUS "Downloading ${filename} from ${url}...")
        file(DOWNLOAD ${url} ${DOWNLOAD_FILE})
    else()
        message(STATUS "File ${filename} already exists, skipping download.")
    endif()
endfunction()


#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h" "stb_rect_pack.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" "stb_truetype.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "stb_image.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "stb_image_write.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h" "raygui.h")

#add_subdirectory(dawn)
set(RG_SOURCES
    "src/InitWindow.cpp"
    "src/raygpu.cpp"
    "src/shapes.cpp"
    "src/shader_parse.cpp"
    "src/stb_impl.c"
    "src/sinfl_impl.c"
    "src/cgltf_impl.c"
    "src/rtext.c"
    "src/rshapes.cpp"
    "src/pipeline.cpp"
    "src/models.cpp"
)

add_library(raygpu ${RG_SOURCES})
if(CMAKE_BUILD_TYPE MATCHES Debug)
    foreach(source_file IN LISTS RG_SOURCES)
        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g -fsanitize=address")
    endforeach()
endif()
if(CMAKE_BUILD_TYPE MATCHES Release)
    foreach(source_file IN LISTS RG_SOURCES)
        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g")
    endforeach()
endif()
SET_SOURCE_FILES_PROPERTIES("src/stb_impl.c" PROPERTIES COMPILE_FLAGS -w)
SET_SOURCE_FILES_PROPERTIES("src/rtext.c" PROPERTIES COMPILE_FLAGS -w)
SET_SOURCE_FILES_PROPERTIES("src/cgltf_impl.c" PROPERTIES COMPILE_FLAGS -w)
target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" "${CMAKE_BINARY_DIR}/dl" "${CMAKE_CURRENT_LIST_DIR}/external")
if(EMSCRIPTEN)
target_link_libraries(raygpu PUBLIC
    emdawnwebgpu_c
    emdawnwebgpu_cpp
    glfw
    tint_api
    tint_api_common
    tint_lang_core
    tint_lang_wgsl
    tint_lang_wgsl_reader
)
target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}dawn/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/src")
else()
target_link_libraries(raygpu PUBLIC
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
endif()
target_include_directories(raygpu PUBLIC)
#add_executable(example "src/example.cpp")
target_compile_features(raygpu PUBLIC cxx_std_20)
#target_compile_features(example PUBLIC cxx_std_20)
#target_link_libraries(example PUBLIC raygpu)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()
if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_GLFW=3 -sASYNCIFY=2 -sSTACK_SIZE=16777216 -sALLOW_MEMORY_GROWTH --shell-file ${CMAKE_CURRENT_LIST_DIR}/resources/minimal.html --preload-file ${CMAKE_CURRENT_LIST_DIR}/resources/@resources/")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples")
#add_executable(reference_sample "src/reference_sample.cpp")
#target_link_libraries(reference_sample PUBLIC webgpu_dawn webgpu_cpp dawn_wgpu_utils dawn_internal_config dawn::dawn_glfw glfw raygpu)
