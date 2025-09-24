cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()
option(SUPPORT_SDL2 "Enable windowing with SDL2" OFF)
option(SUPPORT_SDL3 "Enable windowing with SDL3" OFF)
option(SUPPORT_GLFW "Enable windowing with GLFW" OFF)
option(SUPPORT_RGFW "Enable windowing with RGFW" OFF)
option(RAYGPU_BUILD_TESTS "Build test executables for raygpu" OFF)
option(RAYGPU_BUILD_SHARED_LIBRARY "Build shared raygpu library" OFF)
option(RAYGPU_ENABLE_INSTALL "Enable install targets for raygpu" OFF)

if(NOT SUPPORT_WGPU_BACKEND)
    option(SUPPORT_WGPU_BACKEND "Enable the WebGPU backend" OFF)
    option(SUPPORT_WGSL_PARSER "Support WGSL parsing with tint" OFF)
    option(SUPPORT_GLSL_PARSER "Support GLSL parsing with glslang" ON)
    option(SUPPORT_VULKAN_BACKEND "Enable the Vulkan Backend" ON)
else()
    option(SUPPORT_WGPU_BACKEND "Enable the WebGPU backend" ON)
    option(SUPPORT_WGSL_PARSER "Support WGSL parsing with tint" ON)
    option(SUPPORT_GLSL_PARSER "Support GLSL parsing with glslang" OFF)
    option(SUPPORT_VULKAN_BACKEND "Enable the Vulkan Backend" OFF)
endif()

if(NOT MSVC AND NOT EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -fno-exceptions -g")
endif()

if(SANITIZER_FLAGS)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}     -fsanitize=${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=${SANITIZER_FLAGS}")
endif()


function(cst_bundle_libraries output_target)
  function(get_dependencies input_target)
    get_target_property(alias ${input_target} ALIASED_TARGET)
    if(TARGET ${alias})
      set(input_target ${alias})
    endif()
    if(${input_target} IN_LIST all_dependencies)
      return()
    endif()
    list(APPEND all_dependencies ${input_target})

    get_target_property(link_libraries ${input_target} LINK_LIBRARIES)
    foreach(dependency IN LISTS link_libraries)
      if(TARGET ${dependency})
        get_dependencies(${dependency})
      endif()
    endforeach()

    get_target_property(link_libraries ${input_target} INTERFACE_LINK_LIBRARIES)
    foreach(dependency IN LISTS link_libraries)
      if(TARGET ${dependency})
        get_dependencies(${dependency})
      endif()
    endforeach()

    set(all_dependencies ${all_dependencies} PARENT_SCOPE)
  endfunction()

  foreach(input_target IN LISTS ARGN)
    get_dependencies(${input_target})
  endforeach()

  foreach(dependency IN LISTS all_dependencies)
    get_target_property(type ${dependency} TYPE)
    if(${type} STREQUAL "STATIC_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    elseif(${type} STREQUAL "OBJECT_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    endif()
  endforeach()

  add_library(${output_target} STATIC ${all_objects})

  add_dependencies(${output_target} ${ARGN})

endfunction()
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /GR-")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")
endif()

if(NOT SUPPORT_GLFW)
    set(DAWN_USE_GLFW OFF)
    if(NOT SUPPORT_SDL2)
        if(NOT SUPPORT_SDL3)
            if(NOT SUPPORT_RGFW)
                set(DAWN_USE_X11 OFF)
                set(DAWN_USE_WAYLAND OFF)
            endif()
        endif()
    endif()
endif()

if((SUPPORT_WGPU_BACKEND OR SUPPORT_WGSL_PARSER) AND NOT EMSCRIPTEN)
    set(DAWN_IS_IN_USE True)
    
    #set(TINT_BUILD_CMD_TOOLS ON CACHE BOOL "" FORCE)
    #set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    if(NOT SUPPORT_WGPU_BACKEND)
        set(DAWN_ENABLE_VULKAN OFF)
        set(DAWN_ENABLE_NULL OFF)
    endif()
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_D3D12 OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_D3D11 OFF CACHE BOOL "" FORCE)
    set(DAWN_USE_WINDOWS_UI OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_IR_BINARY OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "" FORCE)
    if(NOT APPLE)
        set(TINT_BUILD_MSL_WRITER OFF CACHE BOOL "" FORCE)
    else()
        set(TINT_BUILD_MSL_WRITER ON CACHE BOOL "" FORCE)
    endif()
    if(SUPPORT_GLSL_PARSER)
        set(TINT_BUILD_SPV_READER ON CACHE BOOL "" FORCE)
    else()
        set(TINT_BUILD_SPV_READER OFF CACHE BOOL "" FORCE)
    endif()
    option(DAWN_BUILD_PROTOBUF "" OFF)
    set(TINT_BUILD_SPV_WRITER ON CACHE BOOL "" FORCE)
    set(TINT_BUILD_HLSL_WRITER OFF CACHE BOOL "" FORCE)
    #set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_SPIRV_VALIDATION OFF CACHE BOOL "" FORCE)
    if(EMSCRIPTEN OR NOT RAYGPU_ENABLE_INSTALL)
        set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "Enable install step for Dawn libraries" FORCE)
        set(TINT_ENABLE_INSTALL OFF CACHE BOOL "Enable install step for Tint libraries" FORCE)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
        set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    else()
        set(DAWN_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Dawn libraries" FORCE)
        set(TINT_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Tint libraries" FORCE)
        set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    endif()
    
    if(NOT SUPPORT_WGPU_BACKEND)
        set(DAWN_USE_GLFW OFF)
        if(SUPPORT_GLFW)
            if(RAYGPU_USE_WAYLAND)
                set(GLFW_BUILD_WAYLAND ON)
            endif()
            add_subdirectory("amalgamation/glfw-3.4")
        endif()
    endif()
    if(UNIX AND NOT ANDROID)
        find_package(PkgConfig QUIET)    
        pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client)
        if(WAYLAND_CLIENT_FOUND)
            option(RAYGPU_USE_WAYLAND "" ON)
            set(DAWN_USE_WAYLAND ON)
        endif()

        find_package(X11 QUIET)
        if(X11_FOUND)
            option(RAYGPU_USE_X11 "" ON)
            option(DAWN_USE_X11 "" ON)
        endif()
    endif()
    set(DAWN_BUILD_MONOLITHIC_LIBRARY "STATIC")
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
            URL https://github.com/manuel5975p/dawn_monorepo/archive/refs/tags/chromium/7369-fixed.tar.gz

        )
        FetchContent_MakeAvailable(dawn)
    endif()
else()
    set(DAWN_IS_IN_USE False)
    find_package(PkgConfig QUIET)    
    pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client)
    if(WAYLAND_CLIENT_FOUND)
        option(RAYGPU_USE_WAYLAND "" ON)
        set(DAWN_USE_WAYLAND ON)
    endif()
    find_package(X11 QUIET)
    if(X11_FOUND)
        option(RAYGPU_USE_X11 "" ON)
        option(DAWN_USE_X11 "" ON)
    endif()
endif()


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
if(NOT EMSCRIPTEN AND SUPPORT_GLSL_PARSER AND NOT TARGET glslang)
    option(ENABLE_OPT "Enable glslang spirv optimizations (requires spirv-tools)" OFF)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/amalgamation/glslang)  
endif()

option(SPIRV_REFLECT_STATIC_LIB "" ON)
option(SPIRV_REFLECT_EXECUTABLE "" OFF)
option(SPIRV_REFLECT_EXAMPLES "" OFF)
option(SPIRV_REFLECT_BUILD_TESTS "" OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/amalgamation/SPIRV-Reflect")
if(SUPPORT_SDL2)
    FetchContent_Declare(
        sdl2fetch
        URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.11.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True
    )
    FetchContent_MakeAvailable(sdl2fetch)
endif()
if(SUPPORT_SDL3)
    #find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        
        set(SDL_ALTIVEC OFF)
        set(SDL_ARMNEON OFF)
        set(SDL_ASAN OFF)
        set(SDL_DIALOG OFF)
        set(SDL_GPU OFF)
        set(SDL_ASSEMBLY OFF)
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(SDL_ASSERTIONS "3")
        else()
            set(SDL_ASSERTIONS "disabled")
        endif()
        set(SDL_AVX OFF)
        set(SDL_AVX2 OFF)
        set(SDL_AVX512F OFF)
        set(SDL_BACKGROUNDING_SIGNAL OFF)
        set(SDL_CCACHE OFF)
        set(SDL_CLANG_TIDY OFF)
        set(SDL_CLOCK_GETTIME OFF)
        if(APPLE)
          set(SDL_COCOA ON)
        else()
          set(SDL_COCOA OFF)
        endif()
        set(SDL_DBUS OFF)
        set(SDL_DEPS_SHARED OFF)
        set(SDL_DIRECTX OFF)
        set(SDL_DISKAUDIO OFF)
        set(SDL_DUMMYAUDIO OFF)
        set(SDL_DUMMYCAMERA OFF)
        set(SDL_DUMMYVIDEO OFF)
        set(SDL_EXAMPLES OFF)
        set(SDL_EXAMPLES_LINK_SHARED OFF)
        set(SDL_FOREGROUNDING_SIGNAL OFF)
        set(SDL_GCC_ATOMICS OFF)
        set(SDL_GPU_DXVK OFF)
        set(SDL_HIDAPI OFF)
        set(SDL_HIDAPI_JOYSTICK OFF)
        set(SDL_HIDAPI_LIBUSB OFF)
        set(SDL_HIDAPI_LIBUSB_SHARED OFF)
        set(SDL_IBUS OFF)
        set(SDL_INSTALL OFF)
        set(SDL_INSTALL_TESTS OFF)
        set(SDL_JACK OFF)
        set(SDL_JACK_SHARED OFF)
        set(SDL_KMSDRM OFF)
        set(SDL_KMSDRM_SHARED OFF)
        set(SDL_LASX OFF)
        set(SDL_LIBICONV OFF)
        set(SDL_LIBUDEV OFF)
        set(SDL_LIBURING OFF)
        set(SDL_LSX OFF)
        set(SDL_METAL OFF)
        set(SDL_MMX OFF)
        set(SDL_OFFSCREEN OFF)
        set(SDL_OPENGL OFF)
        set(SDL_OPENGLES OFF)
        set(SDL_OPENVR OFF)
        set(SDL_OSS OFF)
        set(SDL_RENDER_D3D OFF)
        set(SDL_RENDER_D3D11 OFF)
        set(SDL_RENDER_D3D12 OFF)
        set(SDL_RENDER_GPU OFF)
        set(SDL_RENDER_METAL OFF)
        set(SDL_RENDER_VULKAN OFF)
        set(SDL_ROCKCHIP OFF)
        set(SDL_RPATH OFF)
        set(SDL_RPI OFF)
        set(SDL_SNDIO OFF)
        set(SDL_SNDIO_SHARED OFF)
        set(SDL_SSE OFF)
        set(SDL_SSE2 OFF)
        set(SDL_SSE3 OFF)
        set(SDL_SSE4_1 OFF)
        set(SDL_SSE4_2 OFF)
        set(SDL_SYSTEM_ICONV OFF)
        set(SDL_TESTS OFF)
        set(SDL_TESTS_LINK_SHARED OFF)
        set(SDL_UNINSTALL OFF)
        set(SDL_VIRTUAL_JOYSTICK OFF)
        set(SDL_VIVANTE OFF)
        set(SDL_VULKAN OFF)
        set(SDL_WASAPI OFF)
    
        FetchContent_Declare(
            sdl3fetch
            URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.18.tar.gz
            #URL      "${CMAKE_CURRENT_LIST_DIR}/../release-3.2.4.tar.gz"
            DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        set(SDL_STATIC ON)
        set(SDL_SHARED OFF)
        FetchContent_MakeAvailable(sdl3fetch)
    endif()
endif()

if(ANDROID)
    include_directories("${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include")
endif()



set(RG_SOURCES
    "src/InitWindow.c"
    "src/raygpu.c"
    "src/wgsl_parse_lite.c"
    "src/stb_impl.c"
    "src/sinfl_impl.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
    "src/rtext.c"
    "src/rshapes.c"
    
    "src/models.c"
    "src/windows_stuff.c"
    "include/enum_translation.h"
)

if(SUPPORT_VULKAN_BACKEND)
    if(SUPPORT_WGSL_PARSER)
        set(WGVK_BUILD_WGSL_SUPPORT ON)
    endif()
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../WGVK")
        FetchContent_Declare(
            wgvk
            GIT_REPOSITORY "https://github.com/manuel5975p/WGVK"
            GIT_TAG master
            GIT_SHALLOW True
        )
        FetchContent_MakeAvailable(wgvk)
    else()
        add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../WGVK" ${CMAKE_CURRENT_BINARY_DIR}/wgvk-build)
    endif()
    #target_compile_features(wgvk_standalone PRIVATE c_std_23)
    
    if(RAYGPU_BUILD_SHARED_LIBRARY)
        #target_compile_definitions(wgvk_standalone PRIVATE RG_EXPORTS=1)
        if(SUPPORT_WGSL_PARSER)
            #target_compile_definitions(tint_c_api PRIVATE RG_EXPORTS=1)
        endif()
    else()
        #target_compile_definitions(wgvk_standalone PRIVATE RG_STATIC=1)
        if(SUPPORT_WGSL_PARSER)
            #target_compile_definitions(tint_c_api PRIVATE RG_STATIC=1)
        endif()
    endif()
    
    if(SUPPORT_WGSL_PARSER)
        #cst_bundle_libraries(wgvk_full wgvk_standalone tint_lang_wgsl_reader tint_lang_spirv_writer tint_c_api)
        add_executable(wgsl_compiler "src/wgsl_compiler.c")
        target_link_libraries(wgsl_compiler PUBLIC tint_c_api tint_lang_wgsl_inspector tint_lang_wgsl_reader tint_lang_spirv_writer)
    else()
        #cst_bundle_libraries(wgvk_full wgvk_standalone)
    endif()
    
    list(APPEND RG_SOURCES "src/backend_wgpu.c")

    
endif()
if(SUPPORT_WGPU_BACKEND)
    list(APPEND RG_SOURCES
        "src/backend_wgpu.c"
    )
endif()
if(SUPPORT_SDL2)
    list(APPEND RG_SOURCES "src/InitWindow_SDL2.cpp")
    list(APPEND RG_SOURCES "src/sdl2webgpu.c")
endif()
if(SUPPORT_SDL3)
    list(APPEND RG_SOURCES "src/InitWindow_SDL3.c")
    list(APPEND RG_SOURCES "src/sdl3webgpu.c")
    if(APPLE)
        set_source_files_properties("src/sdl3webgpu.c"
            PROPERTIES
            LANGUAGE OBJC
        )
    endif()
endif()
if(SUPPORT_RGFW)
    list(APPEND RG_SOURCES "src/InitWindow_RGFW.cpp")
    list(APPEND RG_SOURCES "src/rgfwwebgpu.c")
endif()
if(SUPPORT_GLFW)
    list(APPEND RG_SOURCES "src/InitWindow_GLFW.c")
    list(APPEND RG_SOURCES "src/glfw3webgpu.c")
    if(SUPPORT_WGPU_BACKEND)
        if(APPLE)
            set_source_files_properties("src/glfw3webgpu.c"
                PROPERTIES
                LANGUAGE OBJC
            )
        endif()
    endif()
endif()
if(SUPPORT_RGFW)
    list(APPEND RG_SOURCES "src/rgfw_impl.c")
endif()
if (NOT EMSCRIPTEN)
    list(APPEND RG_SOURCES "src/glsl_support.cpp")
endif()

if(DAWN_ENABLE_METAL AND SUPPORT_WGPU_BACKEND)
    set(RAYGPU_USE_METAL ON)
endif()

set(raygpu_core_library_name raygpu_core)
if(RAYGPU_BUILD_SHARED_LIBRARY)
    add_library(${raygpu_core_library_name} SHARED ${RG_SOURCES})
    target_compile_definitions(${raygpu_core_library_name} PRIVATE RG_EXPORTS=1)
else()
    add_library(${raygpu_core_library_name} STATIC ${RG_SOURCES})
    target_compile_definitions(${raygpu_core_library_name} PUBLIC RG_STATIC=1)
endif()
if(RAYGPU_USE_X11)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC RAYGPU_USE_X11=1)
    target_link_libraries(${raygpu_core_library_name} PUBLIC X11::X11)
endif()
if(RAYGPU_USE_WAYLAND)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC RAYGPU_USE_WAYLAND=1)
endif()
if(RAYGPU_USE_METAL)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC RAYGPU_USE_METAL=1)
endif()
if(SUPPORT_SDL2)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_SDL2=1)
endif()
if(SUPPORT_VULKAN_BACKEND)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_VULKAN_BACKEND=1)
endif()
if(SUPPORT_WGPU_BACKEND)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_WGPU_BACKEND=1)
endif()
if(SUPPORT_SDL3)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_SDL3=1)
endif()
if(SUPPORT_GLFW)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_GLFW=1)
endif()
if(SUPPORT_RGFW)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_RGFW=1)
endif()
if(SUPPORT_GLSL_PARSER)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_GLSL_PARSER=1)
endif()
if(SUPPORT_WGSL_PARSER)
    if(EMSCRIPTEN)
        target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_WGSL_PARSER=1)
    else()
        target_compile_definitions(${raygpu_core_library_name} PUBLIC SUPPORT_WGSL_PARSER=1)
    endif()
endif()
if(MSVC)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC NOMINMAX=1)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC _USE_MATH_DEFINES=1)
endif()

if(NOT MSVC AND NOT EMSCRIPTEN)
    #foreach(source_file IN LISTS RG_SOURCES)
    #    set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-fno-inline")
    #endforeach()
    #if(CMAKE_BUILD_TYPE MATCHES Debug)
    #    foreach(source_file IN LISTS RG_SOURCES)
    #        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-fsanitize=address,undefined")
    #    endforeach()
    #endif()
    #if(CMAKE_BUILD_TYPE MATCHES Release)
    #    foreach(source_file IN LISTS RG_SOURCES)
    #        set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g")
    #    endforeach()
    #endif()
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

target_include_directories(${raygpu_core_library_name} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>  
    $<INSTALL_INTERFACE:include/raygpu>
)
set(raygpu_linked_deps "")

if(EMSCRIPTEN)
    if(TARGET emdawnwebgpu_c)
    list(APPEND raygpu_linked_deps
        emdawnwebgpu_c
        emdawnwebgpu_cpp
    )
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --use-port=emdawnwebgpu")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-port=emdawnwebgpu")
    endif()
    #if(SUPPORT_WGSL_PARSER)
    #    list(APPEND raygpu_linked_deps tint_api)
    #ndif()
    if(SUPPORT_SDL2)
        list(APPEND raygpu_linked_deps SDL2::SDL2)
    endif()
    if(SUPPORT_SDL3)
        list(APPEND raygpu_linked_deps SDL3::SDL3)
    endif()
    #target_include_directories(${raygpu_core_library_name} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/dawn/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/src")
else()
    if(SUPPORT_GLSL_PARSER)
        list(APPEND raygpu_linked_deps glslang)
    endif()
    if(SUPPORT_VULKAN_BACKEND)
        target_include_directories(${raygpu_core_library_name} PRIVATE "amalgamation/vulkan_headers/include")
        list(APPEND raygpu_linked_deps wgvk)
    else()
        list(APPEND raygpu_linked_deps webgpu_dawn)
    endif()
    if(SUPPORT_WGSL_PARSER AND NOT EMSCRIPTEN)
        list(APPEND raygpu_linked_deps tint_lang_wgsl_reader tint_lang_spirv_writer)
    endif()
    if(SUPPORT_SDL2)
        list(APPEND raygpu_linked_deps SDL2::SDL2)
    endif()
    if(SUPPORT_SDL3)
        list(APPEND raygpu_linked_deps SDL3::SDL3)
    endif()
    if(SUPPORT_GLFW)
        if(NOT DAWN_IS_IN_USE)
            find_package(glfw QUIET)
            if(NOT glfw_FOUND)
                add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/amalgamation/glfw-3.4")
            endif()
        endif()
        list(APPEND raygpu_linked_deps glfw)
    endif()
    if(SUPPORT_GLSL_PARSER)
        list(APPEND raygpu_linked_deps glslang)
    endif()
endif()
list(APPEND raygpu_linked_deps spirv-reflect-static)

target_link_libraries(${raygpu_core_library_name} PRIVATE ${raygpu_linked_deps})
if(NOT EMSCRIPTEN) 
    target_link_libraries(${raygpu_core_library_name} PUBLIC wgvk)
endif()
#add_executable(example "src/example.cpp")
target_compile_features(${raygpu_core_library_name} PUBLIC cxx_std_20)
if(NOT MSVC)
    target_compile_options(${raygpu_core_library_name} PUBLIC "-Wno-gnu-anonymous-struct" "-Wno-nested-anon-types")
    if(NOT ENABLE_RTTI)
    target_compile_options(${raygpu_core_library_name} PRIVATE "-fno-rtti")
    endif()
endif()

#target_compile_features(example PUBLIC cxx_std_20)
#target_link_libraries(example PUBLIC raygpu)
#if(CMAKE_BUILD_TYPE MATCHES Debug)
if(RAYGPU_USE_LLD)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
endif()

#endif()
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()
if(WIN32)
    if(NOT MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        if(RAYGPU_BUILD_SHARED_LIBRARY)
            target_link_options(${raygpu_core_library_name} PRIVATE "-static-libgcc" "-static-libstdc++")
        endif()
    endif()
endif()


if(RAYGPU_BUILD_TESTS)
    add_executable(hash_map_test "src/test/hash_map_test.c")
    add_executable(test_cc "src/test/test_cc.c")
    target_include_directories(hash_map_test PUBLIC "include")
    target_include_directories(test_cc PUBLIC "include")
endif()

set(EXPORT_RG_TARGETS ${raygpu_core_library_name})
if(SUPPORT_GLFW)
    list(APPEND EXPORT_RG_TARGETS glfw)
endif()
if(SUPPORT_SDL3)
    list(APPEND EXPORT_RG_TARGETS SDL3::SDL3)
endif()

cst_bundle_libraries(raygpu ${raygpu_core_library_name} ${raygpu_linked_deps})

if(DEFINED ENV{HOMEBREW_PREFIX})
    # Construct the RPATH using the environment variable
    set(HOMEBREW_LIB_RPATH "$ENV{HOMEBREW_PREFIX}/lib")
    # Add the constructed RPATH to the target
    target_link_options(${raygpu_core_library_name} PUBLIC "-Wl,-rpath,${HOMEBREW_LIB_RPATH}")
    message(STATUS "HOMEBREW_PREFIX detected. Adding ${HOMEBREW_LIB_RPATH} to RPATH for ${raygpu_core_library_name}.")
endif()


# ----- Install targets for raygpu -----
if(RAYGPU_ENABLE_INSTALL)
  include(CMakePackageConfigHelpers)

  # Exported target names
  set(RAYGPU_INSTALL_EXPORT RaygpuTargets)
  set(RAYGPU_NAMESPACE raygpu::)

  # 1) Install libraries
  install(
    TARGETS
      raygpu
    EXPORT ${RAYGPU_INSTALL_EXPORT}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include/raygpu
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include/raygpu
    FILES_MATCHING
      PATTERN "*.h" PATTERN "*.hpp"
  )

  set(RAYGPU_CONFIG_INSTALL_DIR lib/cmake/raygpu)

  configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/raygpuConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfig.cmake"
    INSTALL_DESTINATION ${RAYGPU_CONFIG_INSTALL_DIR}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfigVersion.cmake"
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfigVersion.cmake"
    DESTINATION ${RAYGPU_CONFIG_INSTALL_DIR}
  )

  install(
    EXPORT ${RAYGPU_INSTALL_EXPORT}
    NAMESPACE ${RAYGPU_NAMESPACE}
    DESTINATION ${RAYGPU_CONFIG_INSTALL_DIR}
  )

  export(
    EXPORT ${RAYGPU_INSTALL_EXPORT}
    NAMESPACE ${RAYGPU_NAMESPACE}
    FILE "${CMAKE_CURRENT_BINARY_DIR}/raygpuTargets.cmake"
  )
endif()


add_subdirectory("examples")
if(RAYGPU_GENERATE_PYTHON_BINDINGS)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG QUIET)
    if (NOT pybind11_FOUND)
        FetchContent_Declare(pybind11fetch
            URL https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        FetchContent_MakeAvailable(pybind11fetch)
    endif()
    pybind11_add_module(
        pyraygpu
        bindings/python/pyraygpu.cpp
    )
    target_link_libraries(pyraygpu PUBLIC raygpu)
endif()



message(STATUS "")
message(STATUS "------ CONFIGURATION OVERVIEW -------")
message(STATUS "")
message(STATUS "SUPPORT_VULKAN_BACKEND = ${SUPPORT_VULKAN_BACKEND}")
message(STATUS " SUPPORT_WGPU_BACKEND  = ${SUPPORT_WGPU_BACKEND}")
message(STATUS "  SUPPORT_GLSL_PARSER  = ${SUPPORT_GLSL_PARSER}")
message(STATUS "  SUPPORT_WGSL_PARSER  = ${SUPPORT_WGSL_PARSER}")
message(STATUS "      SUPPORT_GLFW     = ${SUPPORT_GLFW}")
message(STATUS "      SUPPORT_RGFW     = ${SUPPORT_RGFW}")
message(STATUS "      SUPPORT_SDL2     = ${SUPPORT_SDL2}")
message(STATUS "      SUPPORT_SDL3     = ${SUPPORT_SDL3}")
if(NOT SUPPORT_SDL2 AND NOT SUPPORT_SDL3 AND NOT SUPPORT_GLFW AND NOT SUPPORT_RGFW)
    message(STATUS "")
    message(STATUS "ALL windowing libraries are disabled! No window will ever open up")
endif()
message(STATUS "")
message(STATUS "------ CONFIGURATION OVERVIEW -------")
message(STATUS "")
