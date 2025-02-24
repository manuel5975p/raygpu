cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
set(SUPPORT_WGSL_PARSER ON CACHE BOOL "Support wgsl parsing for uniform and attribute locations" FORCE)
option(SUPPORT_GLSL_PARSER OFF)
option(SUPPORT_SDL2 OFF)
option(SUPPORT_SDL3 OFF)
option(SUPPORT_GLFW "" OFF)
option(SUPPORT_VULKAN_BACKEND OFF)
option(SUPPORT_WGPU_BACKEND "" ON)
message(STATUS "SUPPORT_WGPU_BACKEND: ${SUPPORT_WGPU_BACKEND}")

#if(EMSCRIPTEN)
#    set(SUPPORT_SDL2 OFF CACHE BOOL "Support the SDL2 windowing backend" FORCE)
#endif()
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

if(NOT SUPPORT_GLFW)
    set(DAWN_USE_GLFW OFF)
    if(NOT SUPPORT_SDL2)
        if(NOT SUPPORT_SDL3)
            set(DAWN_USE_X11 OFF)
            set(DAWN_USE_WAYLAND OFF)
        endif()
    endif()
endif()
if(SUPPORT_WGPU_BACKEND OR SUPPORT_WGSL_PARSER)
    set(DAWN_IS_IN_USE True)
    set(TINT_BUILD_CMD_TOOLS ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_MSL_WRITER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_HLSL_WRITER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_TESTS ON CACHE BOOL "" FORCE)
    set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Dawn libraries")
    set(TINT_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Tint libraries")
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/dawn/CMakeLists.txt")
        # Use the local dawn directory
        add_subdirectory("dawn")
        set(dawn_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/dawn")
    else()
        # FetchContent to download dawn if not present locally
        include(FetchContent)
        set(FETCHCONTENT_QUIET FALSE)
        FetchContent_Declare(
            dawn
            URL https://github.com/manuel5975p/dawn_monorepo/archive/refs/tags/v7011.tar.gz

        )
        FetchContent_MakeAvailable(dawn)
    endif()
else()
    set(DAWN_IS_IN_USE False)
endif()
#set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
#set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
#set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
#set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
#set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
#set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
#set(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "" FORCE)
#set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
#set(ENABLE_PCH OFF CACHE BOOL "" FORCE)
#set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
#set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
#set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
#set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)

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
if(NOT EMSCRIPTEN AND SUPPORT_GLSL_PARSER)
    option(ENABLE_OPT "Enable glslang spirv optimizations (requires spirv-tools)" OFF)
    FetchContent_Declare(
        khrGlslang
        URL      https://github.com/KhronosGroup/glslang/archive/refs/tags/15.1.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    if(NOT DAWN_IS_IN_USE)
        FetchContent_MakeAvailable(khrGlslang)
    endif()
    option(SPIRV_REFLECT_STATIC_LIB "" ON)
    option(SPIRV_REFLECT_EXECUTABLE "" OFF)
    option(SPIRV_REFLECT_EXAMPLES "" OFF)
    option(SPIRV_REFLECT_BUILD_TESTS "" OFF)
    FetchContent_Declare(
        khrSpirvReflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
        GIT_SHALLOW TRUE
        GIT_TAG vulkan-sdk-1.4.304
    )
    FetchContent_MakeAvailable(khrSpirvReflect)
endif()
if(SUPPORT_SDL2)
    FetchContent_Declare(
        sdl2fetch
        URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.11.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    FetchContent_MakeAvailable(sdl2fetch)
endif()
if(SUPPORT_SDL3)
    set(SDL3_LIBURING OFF CACHE BOOL "Enable SDL liburing support" FORCE)
    FetchContent_Declare(
        sdl3fetch
        URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.4.tar.gz
        #URL      "${CMAKE_CURRENT_LIST_DIR}/../SDL-release-3.2.2.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    set(SDL_STATIC ON)
    set(SDL_SHARED OFF)
    set(SDL3_LIBURING OFF CACHE BOOL "Enable SDL liburing support" FORCE)
    FetchContent_MakeAvailable(sdl3fetch)
endif()
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h" "stb_rect_pack.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" "stb_truetype.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "stb_image.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "stb_image_write.h")
#download_file_if_not_exists("https://raw.githubusercontent.com/raysan5/raygui/refs/heads/master/src/raygui.h" "raygui.h")

if(ANDROID)
include_directories("${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include")
endif()


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
    
    "src/models.cpp"
    "src/windows_stuff.c"
)
if(SUPPORT_VULKAN_BACKEND)
    list(APPEND RG_SOURCES "src/backend_vulkan/vulkan_textures.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/vulkan_pipeline.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/backend_vulkan.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/wgvk.cpp")
endif()
if(SUPPORT_WGPU_BACKEND)
    list(APPEND RG_SOURCES
        "src/backend_wgpu.cpp"
        "src/pipeline.cpp"
    )
endif()
if(SUPPORT_SDL2)
    list(APPEND RG_SOURCES "src/InitWindow_SDL2.cpp")
    list(APPEND RG_SOURCES "src/sdl2webgpu.c")
endif()
if(SUPPORT_SDL3)
    list(APPEND RG_SOURCES "src/InitWindow_SDL3.cpp")
    list(APPEND RG_SOURCES "src/sdl3webgpu.cpp")
endif()
if(SUPPORT_GLFW)
    list(APPEND RG_SOURCES "src/InitWindow_GLFW.cpp")
    if(SUPPORT_WGPU_BACKEND)
        list(APPEND RG_SOURCES "src/dawn_glfwrip.cpp")
    endif()
endif()
if (NOT EMSCRIPTEN)
    list(APPEND RG_SOURCES "src/glsl_support.cpp")
endif()
if(DAWN_ENABLE_METAL)
    list(APPEND RG_SOURCES "src/utils_metal.mm")
endif()


set(SANITIZER_FLAGS "-fsanitize=address")
add_library(raygpu ${RG_SOURCES})

if(DAWN_USE_X11)
    target_compile_definitions(raygpu PUBLIC RAYGPU_USE_X11=1)
endif()
if(DAWN_USE_WAYLAND)
    target_compile_definitions(raygpu PUBLIC RAYGPU_USE_WAYLAND=1)
endif()
if(SUPPORT_SDL2)
    target_compile_definitions(raygpu PUBLIC SUPPORT_SDL2=1)
endif()
if(SUPPORT_VULKAN_BACKEND)
    target_compile_definitions(raygpu PUBLIC SUPPORT_VULKAN_BACKEND=1)
endif()
if(SUPPORT_WGPU_BACKEND)
    target_compile_definitions(raygpu PUBLIC SUPPORT_WGPU_BACKEND=1)
endif()
if(SUPPORT_SDL3)
    target_compile_definitions(raygpu PUBLIC SUPPORT_SDL3=1)
endif()
if(SUPPORT_GLFW)
    target_compile_definitions(raygpu PUBLIC SUPPORT_GLFW=1)
endif()
if(SUPPORT_GLSL_PARSER)
    target_compile_definitions(raygpu PUBLIC SUPPORT_GLSL_PARSER=1)
endif()
if(SUPPORT_WGSL_PARSER)    
    target_compile_definitions(raygpu PUBLIC SUPPORT_WGSL_PARSER=1)
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
#target_include_directories(raygpu PUBLIC include/)
target_include_directories(raygpu PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>  
    $<INSTALL_INTERFACE:include/raygpu>
)
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
    if(SUPPORT_SDL2)
        target_link_libraries(raygpu PUBLIC SDL2::SDL2)
    endif()
    if(SUPPORT_SDL3)
        target_link_libraries(raygpu PUBLIC SDL3::SDL3)
    endif()
    #target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}/dawn/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/src")
else()
if(SUPPORT_VULKAN_BACKEND)
    target_link_libraries(raygpu PUBLIC vulkan glslang)
else()
    target_link_libraries(raygpu PUBLIC
        webgpu_dawn
        tint_api
        #webgpu_cpp
        #tint_api_common
        #tint_lang_wgsl_reader
        #dawn_internal_config
        #tint_lang_wgsl
        #tint_api
        #glslang
    
        #dawn::dawn_native
        #dawn::dawn_common
        #dawn::dawn_system_utils
        #dawn::dawn_wgpu_utils
        #dawn::dawn_test_utils
        #dawn::dawn_proc
        
        #target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}/dawn/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/src")
    )
    # target_include_directories(raygpu PUBLIC
    #     $<BUILD_INTERFACE:${dawn_SOURCE_DIR}/include/>  
    #     $<BUILD_INTERFACE:${dawn_SOURCE_DIR}/src>
    #     $<BUILD_INTERFACE:${dawn_SOURCE_DIR}>
    #     $<INSTALL_INTERFACE:include/raygpu/dawn/include>
    #     $<INSTALL_INTERFACE:include/raygpu/dawn/src>
    #     $<INSTALL_INTERFACE:include/raygpu/dawn/>
    # )

endif()
if(SUPPORT_WGSL_PARSER)
    target_link_libraries(raygpu PUBLIC tint_api ) #tint_lang_wgsl_reader tint_lang_wgsl_inspector tint_lang_wgsl_resolver tint_lang_spirv_writer)
    #target_include_directories(raygpu PUBLIC  
    #    $<BUILD_INTERFACE:${dawn_SOURCE_DIR}/include/>  
    #    $<BUILD_INTERFACE:${dawn_SOURCE_DIR}/src>
    #    $<BUILD_INTERFACE:${dawn_SOURCE_DIR}>
    #    $<INSTALL_INTERFACE:include/raygpu/dawn/include>
    #    $<INSTALL_INTERFACE:include/raygpu/dawn/src>
    #    $<INSTALL_INTERFACE:include/raygpu/dawn/>
    #)
endif()

if(SUPPORT_SDL2)
    target_link_libraries(raygpu PUBLIC SDL2::SDL2)
endif()
if(SUPPORT_SDL3)
    target_link_libraries(raygpu PUBLIC SDL3::SDL3)
endif()
if(SUPPORT_GLFW)
    target_link_libraries(raygpu PUBLIC glfw)
endif()
if(SUPPORT_GLSL_PARSER)
    target_link_libraries(raygpu PUBLIC glslang spirv-reflect-static)
endif()
endif()

#add_executable(example "src/example.cpp")
target_compile_features(raygpu PUBLIC cxx_std_20)
if(NOT MSVC)
    target_compile_options(raygpu PUBLIC "-Wno-gnu-anonymous-struct" "-Wno-nested-anon-types")
endif()
#target_compile_features(example PUBLIC cxx_std_20)
#target_link_libraries(example PUBLIC raygpu)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()

add_subdirectory("examples")

set(EXPORT_RG_TARGETS raygpu)
if(SUPPORT_GLFW)
    list(APPEND EXPORT_RG_TARGETS glfw)
endif()
if(SUPPORT_SDL3)
    list(APPEND EXPORT_RG_TARGETS SDL3::SDL3)
endif()
if(NOT SUPPORT_VULKAN_BACKEND)
install(
    TARGETS ${EXPORT_RG_TARGETS}
    EXPORT DawnInstall
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
    EXPORT DawnInstall
    FILE raygpuTargets.cmake
    NAMESPACE raygpu::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
)


install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/raygpu"
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/raygpuConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
)
install(
    EXPORT DawnInstall
    FILE raygpuConfig.cmake
    NAMESPACE raygpu::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
)
endif()

#install(TARGETS raygpu)
#file(GLOB_RECURSE tint_wgsl_src 
#    "dawn/src/tint/lang/wgsl/ast/*.cc"
#    "dawn/src/tint/lang/wgsl/reader/*.cc"
#    "dawn/src/tint/lang/wgsl/common/*.cc"
#    "dawn/src/tint/lang/wgsl/inspector/*.cc"
#    "dawn/src/tint/lang/wgsl/intrinsic/*.cc"
#    "dawn/src/tint/lang/wgsl/resolver/*.cc"
#    "dawn/src/tint/lang/wgsl/sem/*.cc"
#    "dawn/src/tint/lang/wgsl/ir/*.cc"
#)
#
## Convert the file to an absolute path
#get_filename_component(READER_BENCH_PATH
#    "${CMAKE_SOURCE_DIR}/dawn/src/tint/lang/wgsl/reader/reader_bench.cc" ABSOLUTE)
#
## Remove it from the list
#list(REMOVE_ITEM tint_wgsl_src "${READER_BENCH_PATH}")
#add_library(tint_full ${tint_wgsl_src})
#target_include_directories(tint_full PUBLIC  
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dawn/include/>  
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dawn/src>
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dawn/>
#    $<INSTALL_INTERFACE:include/raygpu/dawn/include>
#    $<INSTALL_INTERFACE:include/raygpu/dawn/src>
#    $<INSTALL_INTERFACE:include/raygpu/dawn/>
#)
#include(CMakePackageConfigHelpers)
#

# Export the targets for use in a CMake package


## Generate and install a version file
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfigVersion.cmake"
#    COMPATIBILITY AnyNewerVersion
#    VERSION 1.0
#)
#
# Configure the config file

#
## Install the config and version files
#install(FILES
#    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfig.cmake"
#    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfigVersion.cmake"
#    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
#)
