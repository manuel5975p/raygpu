cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)

option(SUPPORT_SDL2 "Enable windowing with SDL2" OFF)
option(SUPPORT_SDL3 "Enable windowing with SDL3" OFF)
option(SUPPORT_GLFW "Enable windowing with GLFW" OFF)
option(SUPPORT_RGFW "Enable windowing with RGFW" OFF)
option(RAYGPU_BUILD_TESTS "Build test executables for raygpu" OFF)
option(RAYGPU_BUILD_SHARED_LIBRARY "Build shared raygpu library" OFF)
option(RAYGPU_ENABLE_INSTALL "(EXPERIMENTAL!) Enable install targets for raygpu" OFF)

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
    if(SANITIZER_FLAGS)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}     -fsanitize=${SANITIZER_FLAGS}")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=${SANITIZER_FLAGS}")
    endif()
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -fno-exceptions -g")
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

if(SUPPORT_WGPU_BACKEND OR SUPPORT_WGSL_PARSER)
    set(DAWN_IS_IN_USE True)
    
    #set(TINT_BUILD_CMD_TOOLS ON CACHE BOOL "" FORCE)
    #set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_D3D12 OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_D3D11 OFF CACHE BOOL "" FORCE)
    set(DAWN_USE_WINDOWS_UI OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_VALIDATOR OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_MSL_WRITER OFF CACHE BOOL "" FORCE)
    if(SUPPORT_GLSL_PARSER)
        set(TINT_BUILD_SPV_READER ON CACHE BOOL "" FORCE)
    else()
        set(TINT_BUILD_SPV_READER OFF CACHE BOOL "" FORCE)
    endif()
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
            add_subdirectory("amalgamation/glfw-3.4")
        endif()
    endif()
    set(DAWN_BUILD_MONOLITHIC_LIBRARY "SHARED")
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
            URL https://github.com/manuel5975p/dawn_monorepo/archive/refs/tags/v7188.tar.gz

        )
        FetchContent_MakeAvailable(dawn)
    endif()
else()
    set(DAWN_IS_IN_USE False)
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
if(NOT EMSCRIPTEN AND SUPPORT_GLSL_PARSER)
    option(ENABLE_OPT "Enable glslang spirv optimizations (requires spirv-tools)" OFF)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/amalgamation/glslang)
    #FetchContent_Declare(
    #    khrGlslang
    #    URL      https://github.com/KhronosGroup/glslang/archive/refs/tags/15.1.0.tar.gz
    #    DOWNLOAD_EXTRACT_TIMESTAMP True
    #)
    #if(NOT DAWN_IS_IN_USE)
        #FetchContent_MakeAvailable(khrGlslang)
    #endif()    
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
        set(SDL_LIBURING OFF CACHE BOOL "Enable SDL liburing support" FORCE)
        FetchContent_Declare(
            sdl3fetch
            URL      https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.4.tar.gz
            #URL      "${CMAKE_CURRENT_LIST_DIR}/../release-3.2.4.tar.gz"
            DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        set(SDL_STATIC ON)
        set(SDL_SHARED OFF)
        set(SDL_LIBURING OFF CACHE BOOL "Enable SDL liburing support" FORCE)
        FetchContent_MakeAvailable(sdl3fetch)
    endif()
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
    "include/enum_translation.h"
)
if(SUPPORT_VULKAN_BACKEND)
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
    if(SUPPORT_WGSL_PARSER)
        set(WGVK_BUILD_WGSL_SUPPORT ON)
        #add_library(tint_c_api STATIC "src/backend_vulkan/tint_c_api.cpp")
        #target_include_directories(tint_c_api PRIVATE "include" "amalgamation/vulkan_headers/include")
        
        #target_compile_features(tint_c_api PRIVATE cxx_std_23)
        #target_link_libraries(tint_c_api PUBLIC tint_lang_wgsl_reader tint_lang_wgsl_inspector)
    endif()
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
        target_link_libraries(wgsl_compiler PUBLIC tint_lang_wgsl_inspector tint_c_api tint_lang_wgsl_reader tint_lang_spirv_writer)
    else()
        #cst_bundle_libraries(wgvk_full wgvk_standalone)
    endif()
    #target_include_directories(wgvk_standalone PUBLIC "include" "amalgamation/vulkan_headers/include")
    #target_compile_definitions(wgvk_standalone PUBLIC SUPPORT_VULKAN_BACKEND=1)

    list(APPEND RG_SOURCES "src/backend_vulkan/vulkan_textures.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/vulkan_pipeline.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/vulkan_raytracing.cpp")
    list(APPEND RG_SOURCES "src/backend_vulkan/backend_vulkan.cpp")

    
endif()
if(SUPPORT_WGPU_BACKEND)
    list(APPEND RG_SOURCES
        "src/backend_wgpu.cpp"
        #"src/wgvk_to_wgpu.cpp"
        "src/pipeline.cpp"
    )
endif()
if(SUPPORT_SDL2)
    list(APPEND RG_SOURCES "src/InitWindow_SDL2.cpp")
    list(APPEND RG_SOURCES "src/sdl2webgpu.c")
endif()
if(SUPPORT_SDL3)
    list(APPEND RG_SOURCES "src/InitWindow_SDL3.cpp")
    if(SUPPORT_WGPU_BACKEND)
        list(APPEND RG_SOURCES "src/sdl3webgpu.cpp")
    endif()
endif()
if(SUPPORT_RGFW)
    list(APPEND RG_SOURCES "src/InitWindow_RGFW.cpp")
    if(SUPPORT_WGPU_BACKEND)
        list(APPEND RG_SOURCES "src/rgfwwebgpu.c")
    endif()
endif()
if(SUPPORT_GLFW)
    list(APPEND RG_SOURCES "src/InitWindow_GLFW.cpp")
    if(SUPPORT_WGPU_BACKEND)
        list(APPEND RG_SOURCES "src/dawn_glfwrip.cpp")
    endif()
endif()
if(SUPPORT_RGFW)
    list(APPEND RG_SOURCES "src/rgfw_impl.c")
endif()
if (NOT EMSCRIPTEN)
    list(APPEND RG_SOURCES "src/glsl_support.cpp")
endif()
if(DAWN_ENABLE_METAL)
    list(APPEND RG_SOURCES "src/utils_metal.mm")
endif()


if(RAYGPU_BUILD_SHARED_LIBRARY)
    add_library(raygpu SHARED ${RG_SOURCES})
    target_compile_definitions(raygpu PRIVATE RG_EXPORTS=1)
else()
    add_library(raygpu STATIC ${RG_SOURCES})
    target_compile_definitions(raygpu PUBLIC RG_STATIC=1)
endif()
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
if(SUPPORT_RGFW)
    target_compile_definitions(raygpu PUBLIC SUPPORT_RGFW=1)
endif()
if(SUPPORT_GLSL_PARSER)
    target_compile_definitions(raygpu PUBLIC SUPPORT_GLSL_PARSER=1)
endif()
if(SUPPORT_WGSL_PARSER)    
    target_compile_definitions(raygpu PUBLIC SUPPORT_WGSL_PARSER=1)
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
#target_include_directories(raygpu PUBLIC include/)
target_include_directories(raygpu PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>  
    $<INSTALL_INTERFACE:include/raygpu>
)
if(EMSCRIPTEN)
    target_link_libraries(raygpu PUBLIC
        emdawnwebgpu_c
        emdawnwebgpu_cpp
    )
    if(SUPPORT_WGSL_PARSER)
        target_link_libraries(raygpu PUBLIC
            tint_api
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
    if(SUPPORT_GLSL_PARSER)
        target_link_libraries(raygpu PUBLIC glslang)
    endif()
    if(SUPPORT_VULKAN_BACKEND)
        target_include_directories(raygpu PUBLIC "amalgamation/vulkan_headers/include")
        target_link_libraries(raygpu PUBLIC wgvk)
    else()
        target_link_libraries(raygpu PUBLIC webgpu_dawn)
    endif()
    if(SUPPORT_WGSL_PARSER)
        target_link_libraries(raygpu PRIVATE tint_lang_wgsl_reader tint_lang_spirv_writer)
    endif()
    if(SUPPORT_SDL2)
        target_link_libraries(raygpu PUBLIC SDL2::SDL2)
    endif()
    if(SUPPORT_SDL3)
        target_link_libraries(raygpu PUBLIC SDL3::SDL3)
    endif()
    if(SUPPORT_GLFW)
        if(NOT DAWN_IS_IN_USE)
            find_package(glfw QUIET)
            if(NOT glfw_FOUND)
                add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/amalgamation/glfw-3.4")
            endif()
        endif()
        target_link_libraries(raygpu PUBLIC glfw)
    endif()
    if(SUPPORT_GLSL_PARSER)
        target_link_libraries(raygpu PRIVATE glslang)
    endif()
    target_link_libraries(raygpu PRIVATE spirv-reflect-static)
endif()

#add_executable(example "src/example.cpp")
target_compile_features(raygpu PUBLIC cxx_std_20)
if(NOT MSVC)
    target_compile_options(raygpu PUBLIC "-Wno-gnu-anonymous-struct" "-Wno-nested-anon-types")
    if(NOT ENABLE_RTTI)
    target_compile_options(raygpu PRIVATE "-fno-rtti")
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
            target_link_options(raygpu PRIVATE "-static-libgcc" "-static-libstdc++")
        endif()
    endif()
endif()

add_subdirectory("examples")
if(RAYGPU_BUILD_TESTS)
    add_executable(hash_map_test "src/test/hash_map_test.c")
    add_executable(test_cc "src/test/test_cc.c")
    target_include_directories(hash_map_test PUBLIC "src/backend_vulkan/")
    target_include_directories(hash_map_test PUBLIC "include")
    target_include_directories(test_cc PUBLIC "include")
endif()

set(EXPORT_RG_TARGETS raygpu)
if(SUPPORT_GLFW)
    list(APPEND EXPORT_RG_TARGETS glfw)
endif()
if(SUPPORT_SDL3)
    list(APPEND EXPORT_RG_TARGETS SDL3::SDL3)
endif()
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
#if(SUPPORT_VULKAN_BACKEND)
#    get_target_property(_inc_dirs wgvk_standalone INTERFACE_INCLUDE_DIRECTORIES)
#    if(_inc_dirs AND NOT _inc_dirs STREQUAL "NOTFOUND")
#      target_include_directories(wgvk_full PUBLIC ${_inc_dirs})
#    endif()
#    
#    # 2) copy all INTERFACE_COMPILE_DEFINITIONS
#    get_target_property(_defs wgvk_standalone INTERFACE_COMPILE_DEFINITIONS)
#    if(_defs AND NOT _defs STREQUAL "NOTFOUND")
#      target_compile_definitions(wgvk_full PUBLIC ${_defs})
#    endif()
#    
#    # 3) copy all INTERFACE_COMPILE_OPTIONS
#    get_target_property(_opts wgvk_standalone INTERFACE_COMPILE_OPTIONS)
#    if(_opts AND NOT _opts STREQUAL "NOTFOUND")
#      target_compile_options(wgvk_full PUBLIC ${_opts})
#    endif()
#endif()
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

#if(NOT SUPPORT_VULKAN_BACKEND AND FALSE)
#install(
#    TARGETS ${EXPORT_RG_TARGETS}
#    EXPORT DawnInstall
#    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
#    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#)
#install(
#    EXPORT DawnInstall
#    FILE raygpuTargets.cmake
#    NAMESPACE raygpu::
#    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
#)
#install(
#    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
#    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/raygpu"
#    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
#)
#configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/raygpuConfig.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/raygpuConfig.cmake"
#    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
#)
#install(
#    EXPORT DawnInstall
#    FILE raygpuConfig.cmake
#    NAMESPACE raygpu::
#    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raygpu"
#)
#endif()

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
