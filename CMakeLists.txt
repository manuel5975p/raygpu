cmake_minimum_required(VERSION 3.25)
include(FetchContent)
project(webgpu_standalone)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
#FetchContent_Declare(
#    dawn
#    URL "${CMAKE_CURRENT_LIST_DIR}/dawn.tar.gz"
#)
#FetchContent_MakeAvailable(dawn)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dawn)
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


download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "stb_image.h")
download_file_if_not_exists("https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "stb_image_write.h")


#add_subdirectory(dawn)
set(RG_SOURCES
    "src/InitWindow.cpp"
    "src/raygpu.cpp"
    "src/shapes.cpp"
    "src/stb_impl.c"
    "src/pipeline.cpp"
)

add_library(raygpu ${RG_SOURCES})
foreach(source_file IN LISTS RG_SOURCES)
    set_source_files_properties(${source_file} PROPERTIES COMPILE_FLAGS "-g")
endforeach()
SET_SOURCE_FILES_PROPERTIES("src/stb_impl.c" PROPERTIES COMPILE_FLAGS -w)
target_include_directories(raygpu PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" "${CMAKE_CURRENT_LIST_DIR}/dawn/include" "${CMAKE_BINARY_DIR}/dl")
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
target_include_directories(raygpu PUBLIC )
add_executable(example "src/example.cpp")
target_compile_features(raygpu PUBLIC cxx_std_20)
target_compile_features(example PUBLIC cxx_std_20)
target_link_libraries(example PUBLIC raygpu)

add_executable(reference_sample "src/reference_sample.cpp")
target_link_libraries(reference_sample PUBLIC webgpu_dawn webgpu_cpp dawn_wgpu_utils dawn_internal_config dawn::dawn_glfw glfw raygpu)
