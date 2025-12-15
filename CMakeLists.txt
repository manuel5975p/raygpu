cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()
option(RAYGPU_GENERATE_PYTHON_BINDINGS "Generate python binding (dynamic library) with pybind" OFF)
option(SUPPORT_SDL3 "Enable windowing with SDL3" OFF)
option(SUPPORT_GLFW "Enable windowing with GLFW" OFF)
option(SUPPORT_RGFW "Enable windowing with RGFW" OFF)
option(RAYGPU_BUILD_TESTS "Build test executables for raygpu" OFF)
option(RAYGPU_BUILD_SHARED_LIBRARY "Build shared raygpu library" OFF)
option(RAYGPU_ENABLE_INSTALL "Enable install targets for raygpu" OFF)

if(EMSCRIPTEN OR SUPPORT_WGPU_BACKEND)
    option(SUPPORT_WGPU_BACKEND "Enable the WebGPU backend" ON)
    option(SUPPORT_WGSL_PARSER "Support WGSL parsing with tint" ON)
    option(SUPPORT_GLSL_PARSER "Support GLSL parsing with glslang" OFF)
    option(SUPPORT_VULKAN_BACKEND "Enable the Vulkan Backend" OFF)
    if(SUPPORT_WGPU_BACKEND)
        option(SUPPORT_TINT_WGSL_PARSER "Use tint to parse WGSL" ON)
    else()
        option(SUPPORT_TINT_WGSL_PARSER "Use tint to parse WGSL" OFF)
    endif()
else()
    option(SUPPORT_WGPU_BACKEND "Enable the WebGPU backend" OFF)
    option(SUPPORT_WGSL_PARSER "Support WGSL parsing with tint" OFF)
    option(SUPPORT_GLSL_PARSER "Support GLSL parsing with glslang" ON)
    option(SUPPORT_VULKAN_BACKEND "Enable the Vulkan Backend" ON)
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
  set(options "")
  set(oneValueArgs TYPE)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
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
  foreach(input_target IN LISTS ARG_UNPARSED_ARGUMENTS)
  get_dependencies(${input_target})
  endforeach()
  foreach(dependency IN LISTS all_dependencies)
  get_target_property(type ${dependency} TYPE)
  if(${type} STREQUAL "STATIC_LIBRARY")
    list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
  elseif(${type} STREQUAL "OBJECT_LIBRARY")
    list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
  elseif(${type} STREQUAL "SHARED_LIBRARY")
    list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
  endif()
  endforeach()
  if(ARG_TYPE)
    add_library(${output_target} ${ARG_TYPE} ${all_objects})
  else()
    add_library(${output_target} ${all_objects})
  endif()
  add_dependencies(${output_target} ${ARG_UNPARSED_ARGUMENTS})
endfunction()


if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /GR-")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")
endif()

set(DAWN_USE_GLFW OFF) # raygpu provides its own WGPUSurface creation mechanism


if(NOT SUPPORT_GLFW AND NOT SUPPORT_SDL3 AND NOT SUPPORT_RGFW)
    set(DAWN_USE_X11 OFF)
    set(DAWN_USE_WAYLAND OFF)
    option(RAYGPU_USE_X11 OFF)
else()
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
    set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
    set(DAWN_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_SPIRV_VALIDATION OFF CACHE BOOL "" FORCE)
    if(EMSCRIPTEN OR NOT RAYGPU_ENABLE_INSTALL)
        set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "Enable install step for Dawn libraries" FORCE)
        set(TINT_ENABLE_INSTALL OFF CACHE BOOL "Enable install step for Tint libraries" FORCE)
        set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
        set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    else()
        set(DAWN_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Dawn libraries" FORCE)
        set(TINT_ENABLE_INSTALL ON CACHE BOOL "Enable install step for Tint libraries" FORCE)
        set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    endif()
    
    
    if(UNIX AND NOT ANDROID AND NOT APPLE)
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
            URL https://github.com/manuel5975p/dawn_monorepo/archive/refs/tags/v7562.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP True
        )
        FetchContent_MakeAvailable(dawn)
    endif()
else()

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
    option(ENABLE_HLSL "Enable HLSL compilation" OFF)
    option(ENABLE_SPVREMAPPER "" OFF)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/amalgamation/glslang)  
endif()

if(SUPPORT_SDL3)
    #find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        
        list(APPEND SDL_DISABLED_FEATURES
            ALTIVEC ARMNEON ASAN DIALOG GPU ASSEMBLY AVX AVX2 AVX512F BACKGROUNDING_SIGNAL
            CCACHE CLANG_TIDY CLOCK_GETTIME DBUS DEPS_SHARED DIRECTX POWER JOYSTICK HAPTIC AUDIO DISKAUDIO DUMMYAUDIO
            DUMMYCAMERA DUMMYVIDEO EXAMPLES EXAMPLES_LINK_SHARED FOREGROUNDING_SIGNAL
            GCC_ATOMICS GPU_DXVK HIDAPI HIDAPI_JOYSTICK HIDAPI_LIBUSB PULSEAUDIO PIPEWIRE RENDER SENSOR HIDAPI_LIBUSB_SHARED
            IBUS INSTALL INSTALL_TESTS JACK JACK_SHARED KMSDRM KMSDRM_SHARED LASX LIBICONV
            LIBUDEV LIBURING LSX METAL MMX OFFSCREEN OPENGL OPENGLES OPENVR OSS RENDER_D3D
            RENDER_D3D11 RENDER_D3D12 RENDER_GPU RENDER_METAL RENDER_VULKAN ROCKCHIP RPATH
            RPI SNDIO SNDIO_SHARED SYSTEM_ICONV TESTS
            TESTS_LINK_SHARED UNINSTALL VIRTUAL_JOYSTICK VIVANTE VULKAN WASAPI
        )

        # Disable all features in the list.
        foreach(FEATURE ${SDL_DISABLED_FEATURES})
            set(SDL_${FEATURE} OFF)
        endforeach()
        # Special case for assertions based on build type.
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(SDL_ASSERTIONS "3")
        else()
            set(SDL_ASSERTIONS "disabled")
        endif()

        # Enable Cocoa on Apple platforms.
        if(APPLE)
            set(SDL_COCOA ON)
        else()
            set(SDL_COCOA OFF)
        endif()
        if(NOT SDL3_URL)
            set(SDL3_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.22.tar.gz")
        endif()
        message(STATUS "Fetching SDL3 from ${SDL3_URL}")
        FetchContent_Declare(
            sdl3fetch
            URL ${SDL3_URL}
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(SDL_STATIC ON)
        set(SDL_SHARED OFF)
        FetchContent_MakeAvailable(sdl3fetch)
    endif()
endif()

if(ANDROID)
    include_directories("${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include")
endif()

if(NOT SUPPORT_WGPU_BACKEND)
    set(DAWN_USE_GLFW OFF)
endif()
if(SUPPORT_GLFW)
    set(GLFW_BUILD_WAYLAND ${RAYGPU_USE_WAYLAND})
    set(GLFW_BUILD_X11 ${RAYGPU_USE_X11})
    add_subdirectory("amalgamation/glfw-3.4")
endif()


set(RG_SOURCES
    "src/InitWindow.c"
    "src/raygpu.c"
    "src/stb_impl.c"
    "src/sinfl_impl.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
    "src/rtext.c"
    "src/rshapes.c"
    "src/models.c"
    "src/windows_stuff.c"
    "src/backend_wgpu.c"
)
if(SUPPORT_TINT_WGSL_PARSER)
    list(APPEND RG_SOURCES "src/shader_parse.cpp")
else()
    list(APPEND RG_SOURCES "src/wgsl_parse_lite.c")
endif()

if(SUPPORT_VULKAN_BACKEND)
    set(WGVK_BUILD_WGSL_SUPPORT ${SUPPORT_WGSL_PARSER})
    
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
endif()
if(APPLE)
    set_source_files_properties("src/InitWindow.c"
        PROPERTIES
        LANGUAGE OBJC
    )
    set_source_files_properties("src/glfw3webgpu.c"
        PROPERTIES
        LANGUAGE OBJC
    )
endif()

if (NOT EMSCRIPTEN)
    list(APPEND RG_SOURCES "src/glsl_support.cpp")
endif()

if(APPLE)
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
target_include_directories(${raygpu_core_library_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/amalgamation/SPIRV-Reflect)
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
    if(SUPPORT_TINT_WGSL_PARSER)
        target_compile_definitions(${raygpu_core_library_name} PRIVATE SUPPORT_TINT_WGSL_PARSER=1)
    endif()
endif()
if(MSVC)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC NOMINMAX=1)
    target_compile_definitions(${raygpu_core_library_name} PUBLIC _USE_MATH_DEFINES=1)
endif()

set(SUPPRESS_WARNINGS_SOURCE_FILES
    "src/stb_impl.c"
    "src/rtext.c"
    "src/cgltf_impl.c"
    "src/msf_gif_impl.c"
)
foreach(file ${SUPPRESS_WARNINGS_SOURCE_FILES})
    if (MSVC)
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "/W0")
    else()
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "-w")
    endif()
endforeach()


target_include_directories(${raygpu_core_library_name} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>  
    $<INSTALL_INTERFACE:include/>
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
    if(SUPPORT_SDL3)
        list(APPEND raygpu_linked_deps SDL3::SDL3)
    endif()
    if(SUPPORT_GLFW)
        list(APPEND raygpu_linked_deps glfw)
    endif()
    if(SUPPORT_GLSL_PARSER)
        list(APPEND raygpu_linked_deps glslang)
    endif()
endif()

target_link_libraries(${raygpu_core_library_name} PUBLIC ${raygpu_linked_deps})

if(NOT SUPPORT_WGPU_BACKEND) 
    target_link_libraries(${raygpu_core_library_name} PUBLIC wgvk)
endif()


if(NOT MSVC)
    target_compile_options(${raygpu_core_library_name} PUBLIC "-Wno-gnu-anonymous-struct" "-Wno-nested-anon-types")
endif()

target_compile_features(${raygpu_core_library_name} PUBLIC cxx_std_20)
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
if(RAYGPU_BUILD_SHARED_LIBRARY OR BUILD_SHARED_LIBS)
  
  cst_bundle_libraries(raygpu ${raygpu_core_library_name} TYPE SHARED ${raygpu_linked_deps})
else()
  cst_bundle_libraries(raygpu ${raygpu_core_library_name} TYPE STATIC ${raygpu_linked_deps})
endif()
# Don't link to raygpu_core at all - raygpu already contains its objects
# Just propagate the interface properties manually:
target_include_directories(raygpu PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>  
    $<INSTALL_INTERFACE:include/>
)
target_include_directories(${raygpu_core_library_name} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/internal_include")
get_target_property(core_compile_defs ${raygpu_core_library_name} INTERFACE_COMPILE_DEFINITIONS)
if(core_compile_defs)
    target_compile_definitions(raygpu PUBLIC ${core_compile_defs})
endif()


target_link_libraries(raygpu INTERFACE raygpu_core)
if((RAYGPU_BUILD_SHARED_LIBRARY OR BUILD_SHARED_LIBS) AND APPLE)
  target_link_libraries(raygpu PUBLIC "-framework Cocoa" "-framework IOKit" "-framework Metal" "-framework QuartzCore")
endif()

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
  if(SUPPORT_GLFW)
    set(GLFW_MAYBE glfw)
  endif()
  
  install(
    TARGETS
      raygpu raygpu_core wgvk glslang ${GLFW_MAYBE}
    EXPORT ${RAYGPU_INSTALL_EXPORT}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING
      PATTERN "raygpu.h" PATTERN "config.h" PATTERN "macros_and_constants.h" PATTERN "mathutils.h"
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
message(STATUS "      SUPPORT_SDL3     = ${SUPPORT_SDL3}")
if(NOT SUPPORT_SDL3 AND NOT SUPPORT_GLFW AND NOT SUPPORT_RGFW)
    message(STATUS "")
    message(STATUS "ALL windowing libraries are disabled! No window will ever open up")
endif()
message(STATUS "")
message(STATUS "------ CONFIGURATION OVERVIEW -------")
message(STATUS "")
