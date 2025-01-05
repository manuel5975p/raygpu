cmake_minimum_required(VERSION 3.25)
include(FetchContent)

project(webgpu_standalone)

# Option to specify supported backends
# Usage: -DSUPPORT_BACKENDS="opengl;vulkan;dx12"
set(SUPPORT_BACKENDS "vulkan" CACHE STRING "Semicolon-separated list of supported backends: opengl, opengles, vulkan, metal, dx11, dx12")

# Validate and process SUPPORT_BACKENDS
set(ALLOWED_BACKENDS opengl opengles vulkan metal dx11 dx12)
set(SELECTED_BACKENDS)
foreach(BACKEND IN LISTS SUPPORT_BACKENDS)
    string(TOLOWER "${BACKEND}" BACKEND_LOWER)
    if(BACKEND_LOWER IN_LIST ALLOWED_BACKENDS)
        list(APPEND SELECTED_BACKENDS "${BACKEND_LOWER}")
    else()
        message(FATAL_ERROR "Unsupported backend specified: ${BACKEND}")
    endif()
endforeach()

# Set Dawn build options based on selected backends
# Reset all backend-related options to OFF
set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "Enable OpenGLES backend" FORCE)
set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "Enable OpenGL backend" FORCE)
set(DAWN_ENABLE_VULKAN OFF CACHE BOOL "Enable Vulkan backend" FORCE)
set(DAWN_ENABLE_METAL OFF CACHE BOOL "Enable Metal backend" FORCE)
set(DAWN_ENABLE_DX11 OFF CACHE BOOL "Enable DirectX 11 backend" FORCE)
set(DAWN_ENABLE_DX12 OFF CACHE BOOL "Enable DirectX 12 backend" FORCE)

# Enable the selected backends
foreach(BACKEND IN LISTS SELECTED_BACKENDS)
    if(BACKEND STREQUAL "opengl")
        set(DAWN_ENABLE_DESKTOP_GL ON CACHE BOOL "Enable OpenGL backend" FORCE)
    elseif(BACKEND STREQUAL "opengles")
        set(DAWN_ENABLE_OPENGLES ON CACHE BOOL "Enable OpenGLES backend" FORCE)
    elseif(BACKEND STREQUAL "vulkan")
        set(DAWN_ENABLE_VULKAN ON CACHE BOOL "Enable Vulkan backend" FORCE)
    elseif(BACKEND STREQUAL "metal")
        set(DAWN_ENABLE_METAL ON CACHE BOOL "Enable Metal backend" FORCE)
    elseif(BACKEND STREQUAL "dx11")
        set(DAWN_ENABLE_DX11 ON CACHE BOOL "Enable DirectX 11 backend" FORCE)
    elseif(BACKEND STREQUAL "dx12")
        set(DAWN_ENABLE_DX12 ON CACHE BOOL "Enable DirectX 12 backend" FORCE)
    endif()
endforeach()

# Common Dawn and Tint configuration
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Add Dawn subdirectory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dawn)

# Optional: Define download function if needed
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

# Example usage (uncomment if needed)
# download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h" "stb_rect_pack.h")
# download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" "stb_truetype.h")
# download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "stb_image.h")
# download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "stb_image_write.h")
# download_file_if_not_exists("https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h" "raygui.h")

# Source files
set(RG_SOURCES
    "src/InitWindow.cpp"
    "src/raygpu.cpp"
    "src/shapes.cpp"
    "src/shader_parse.cpp"
    "src/stb_impl.c"
    "src/sinfl_impl.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
    "src/rtext.c"
    "src/rshapes.cpp"
    "src/pipeline.cpp"
    "src/models.cpp"
)

add_library(raygpu ${RG_SOURCES})

# Apply compile flags based on build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    foreach(source_file IN LISTS RG_SOURCES)
        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g -fsanitize=address")
    endforeach()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    foreach(source_file IN LISTS RG_SOURCES)
        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g")
    endforeach()
endif()

# Suppress warnings for specific files
set(WARNING_SUPPRESS_FILES
    "src/stb_impl.c"
    "src/rtext.c"
    "src/cgltf_impl.c"
)
foreach(file IN LISTS WARNING_SUPPRESS_FILES)
    set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "-w")
endforeach()

# Include directories
target_include_directories(raygpu PUBLIC 
    "${CMAKE_CURRENT_LIST_DIR}/include" 
    "${CMAKE_BINARY_DIR}/dl" 
    "${CMAKE_CURRENT_LIST_DIR}/external"
)

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
    target_include_directories(raygpu PUBLIC 
        "${CMAKE_CURRENT_LIST_DIR}/dawn/include" 
        "${CMAKE_CURRENT_LIST_DIR}/dawn/src"
    )
else()
    target_link_libraries(raygpu PUBLIC
        webgpu_dawn
        webgpu_cpp
        dawn_internal_config
        dawn::dawn_glfw
        glfw
    )
endif()

# Set C++ standard
target_compile_features(raygpu PUBLIC cxx_std_20)

# Apply additional linker flags based on build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} "
        "-sUSE_GLFW=3 "
        "-sASYNCIFY=2 "
        "-sSTACK_SIZE=16777216 "
        "-sINITIAL_MEMORY=128Mb "
        "-sALLOW_MEMORY_GROWTH "
        "--shell-file ${CMAKE_CURRENT_LIST_DIR}/resources/minimal.html "
        "--preload-file ${CMAKE_CURRENT_LIST_DIR}/resources/@resources/"
    )
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line sets the executable to build with the Emscripten HTML template
endif()

# Add examples subdirectory
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples")

# Optional: Define additional executables
# add_executable(example "src/example.cpp")
# target_compile_features(example PUBLIC cxx_std_20)
# target_link_libraries(example PUBLIC raygpu)

# Optional: Reference sample executable
# add_executable(reference_sample "src/reference_sample.cpp")
# target_link_libraries(reference_sample PUBLIC webgpu_dawn webgpu_cpp dawn_wgpu_utils dawn_internal_config dawn::dawn_glfw glfw raygpu)
