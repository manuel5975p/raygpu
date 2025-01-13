cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
set(SUPPORT_WGSL_PARSER ON)
set(SUPPORT_GLSL_PARSER OFF)
set(SUPPORT_SDL2 OFF)
set(SUPPORT_GLFW ON)
if(EMSCRIPTEN)
set(SUPPORT_SDL2 OFF CACHE BOOL "Support the SDL2 windowing backend" FORCE)
endif()
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
#FetchContent_Declare(
#    dawn
#    URL "${CMAKE_CURRENT_LIST_DIR}/dawn.tar.gz"
#)
#FetchContent_MakeAvailable(dawn)
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    if(ENABLE_FSANITIZE_LEAK)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
    endif()
endif()
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_VALIDATOR ON CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "" FORCE)
set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
set(DAWN_ENABLE_OPENGLES ON CACHE BOOL "" FORCE)
set(DAWN_ENABLE_DESKTOP_GL ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dawn)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_OPENGLES ON CACHE BOOL "" FORCE)
set(DAWN_ENABLE_DESKTOP_GL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_VALIDATOR ON CACHE BOOL "" FORCE)
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
if(NOT EMSCRIPTEN)
    FetchContent_Declare(
        khrGlslang
        URL      https://github.com/KhronosGroup/glslang/archive/refs/tags/15.1.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    FetchContent_MakeAvailable(khrGlslang)
    if(SUPPORT_SDL2)
        FetchContent_Declare(
            sdl2fetch
            URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.11.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        FetchContent_MakeAvailable(sdl2fetch)
        FetchContent_Declare(
            sdl2wgpue
            GIT_REPOSITORY      https://github.com/manuel5975p/sdl2webgpu.git
            GIT_TAG main
            #DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        FetchContent_MakeAvailable(sdl2wgpue)
        target_link_libraries(sdl2webgpu PUBLIC webgpu_dawn)
        target_compile_definitions(sdl2webgpu PRIVATE WEBGPU_BACKEND_DAWN)
    endif()
    
endif()
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h" "stb_rect_pack.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" "stb_truetype.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "stb_image.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "stb_image_write.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h" "raygui.h")

#add_subdirectory(dawn)
set(RG_SOURCES
    "src/InitWindow.cpp"
    "src/raygpu.cpp"
    "src/shader_parse.cpp"
    "src/stb_impl.c"
    "src/sinfl_impl.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
    "src/rtext.c"
    "src/rshapes.cpp"
    "src/pipeline.cpp"
    "src/models.cpp"
    "src/windows_stuff.c"
)
if(SUPPORT_SDL2)
    list(APPEND RG_SOURCES "src/InitWindow_SDL.cpp")
endif()
if(SUPPORT_GLFW)
    list(APPEND RG_SOURCES "src/dawn_glfwrip.cpp" "src/InitWindow_GLFW.cpp")
endif()
if (NOT EMSCRIPTEN)
    list(APPEND RG_SOURCES "src/glsl_support.cpp")
endif()
if(DAWN_ENABLE_METAL)
    list(APPEND RG_SOURCES "src/utils_metal.mm")
endif()


set(SANITIZER_FLAGS "-fsanitize=address")
add_library(raygpu ${RG_SOURCES})
if(SUPPORT_SDL2)
    target_compile_definitions(raygpu PUBLIC SUPPORT_SDL2=1)
endif()
if(SUPPORT_GLFW)
    target_compile_definitions(raygpu PUBLIC SUPPORT_GLFW=1)
endif()
if(NOT MSVC AND NOT EMSCRIPTEN)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        foreach(source_file IN LISTS RG_SOURCES)
            set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g ${SANITIZER_FLAGS}")
        endforeach()
    endif()
    if(CMAKE_BUILD_TYPE MATCHES Release)
        foreach(source_file IN LISTS RG_SOURCES)
            set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g")
        endforeach()
    endif()
endif()
set(SUPPRESS_WARNINGS_SOURCE_FILES
    "src/stb_impl.c"
    "src/rtext.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
)
foreach(file ${SUPPRESS_WARNINGS_SOURCE_FILES})
    if (MSVC)
        # Suppress all warnings in MSVC
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "/W0")
    else()
        # Suppress all warnings in GCC/Clang
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "-w")
    endif()
endforeach()
target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" "${CMAKE_BINARY_DIR}/dl" "${CMAKE_CURRENT_LIST_DIR}/external")
if(EMSCRIPTEN)
    target_link_libraries(raygpu PUBLIC
        emdawnwebgpu_c
        emdawnwebgpu_cpp
        #glfw
        #tint_lang_spirv
        #tint_lang_spirv_reader
        #tint_lang_spirv_reader_lower
        #tint_lang_spirv_intrinsic
        #tint_lang_spirv_ir
        #tint_lang_spirv_type
        #tint_lang_spirv_reader_parser
        #glslang
    )
    if(SUPPORT_WGSL_PARSER)
    target_link_libraries(raygpu PUBLIC
        tint_api
        tint_api_common
        tint_lang_core
        tint_lang_wgsl
        tint_lang_wgsl_reader
    )
    endif()
    
    target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}dawn/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/src")
else()
    target_link_libraries(raygpu PUBLIC
        webgpu_dawn
        webgpu_cpp
        #tint_api_common
        #tint_lang_wgsl_reader
        dawn_internal_config
        
        
        #glslang
    
        #dawn::dawn_native
        #dawn::dawn_common
        #dawn::dawn_system_utils
        #dawn::dawn_wgpu_utils
        #dawn::dawn_test_utils
        #dawn::dawn_proc
    )
    if(SUPPORT_SDL2)
    target_link_libraries(raygpu PUBLIC SDL2::SDL2 sdl2webgpu)
    endif()
    if(SUPPORT_GLFW)
    target_link_libraries(raygpu PUBLIC glfw)
    endif()
endif()
target_include_directories(raygpu PUBLIC)
#add_executable(example "src/example.cpp")
target_compile_features(raygpu PUBLIC cxx_std_20)
target_compile_options(raygpu PUBLIC "-Wno-gnu-anonymous-struct" "-Wno-nested-anon-types")
#target_compile_features(example PUBLIC cxx_std_20)
#target_link_libraries(example PUBLIC raygpu)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples")
#add_executable(reference_sample "src/reference_sample.cpp")
#target_link_libraries(reference_sample PUBLIC webgpu_dawn webgpu_cpp dawn_wgpu_utils dawn_internal_config dawn::dawn_glfw glfw raygpu)
