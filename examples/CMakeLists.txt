if(ANDROID)
#add_library(android_glue "${ANDROID_NDK_ROOT}/sources/android/native_app_glue/android_native_app_glue.c")
endif()

function(add_example target_name)
    if(ANDROID)
        add_library(${target_name} SHARED "${target_name}.c" "${ANDROID_NDK_ROOT}/sources/android/native_app_glue/android_native_app_glue.c")
        target_compile_definitions(${target_name} PUBLIC ANDROID PLATFORM_ANDROID __ANDROID_API__=35)
        target_link_libraries(${target_name} PUBLIC android_glue raygpu log android EGL GLESv2 OpenSLES atomic c m dl)
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS "-z noexecstack -z now -z relro")
    else()
        add_executable(${target_name} "${target_name}.c")
        target_link_libraries(${target_name} PUBLIC raygpu)
        if(EMSCRIPTEN)
            target_link_options(${target_name} PRIVATE
                -sUSE_GLFW=3
                -sASYNCIFY=2
                -sSTACK_SIZE=16777216
                -sINITIAL_MEMORY=256Mb
                -sALLOW_MEMORY_GROWTH
                --shell-file   ${CMAKE_SOURCE_DIR}/resources/minimal.html
                --preload-file ${CMAKE_SOURCE_DIR}/resources/@resources/
            )
        endif()
    endif()
endfunction()
function(add_cpp_example target_name)
    if(ANDROID)
    add_library(${target_name} SHARED "${target_name}.cpp")
    target_link_libraries(${target_name} PUBLIC android_glue raygpu log android EGL GLESv2 OpenSLES atomic c m dl)
    else()
    add_executable(${target_name} "${target_name}.cpp")
    if(NOT ENABLE_RTTI)
        target_compile_options(${target_name} PRIVATE "-fno-rtti")
    endif()
    target_link_libraries(${target_name} PUBLIC raygpu)
    if(EMSCRIPTEN)
    target_link_options(${target_name} PRIVATE
        -sUSE_GLFW=3
        -sASYNCIFY=2
        -sSTACK_SIZE=16777216
        -sINITIAL_MEMORY=256Mb
        -sALLOW_MEMORY_GROWTH
        --shell-file   ${CMAKE_SOURCE_DIR}/resources/minimal.html
        --preload-file ${CMAKE_SOURCE_DIR}/resources/@resources/
    )
    endif()
    endif()
endfunction()

add_example(core_window)
add_example(core_resizable)
add_example(core_shapes)
add_example(shaders_basic)
#add_example(core_vsync_fullscreen)
add_example(core_msaa)
#add_example(textures_generate)
add_example(core_camera2d)
#add_example(core_camera3d)
add_example(core_rendertexture)
add_example(pipeline_basic)
#add_example(pipeline_settings)
add_example(input_keys)
add_example(input_multitouch_gesture)
#add_example(input_text)
add_example(models_cube)
#add_example(split_screen_camera3d)
#add_example(core_fonts)
add_example(core_headless)
#add_example(textures_array)
add_example(core_multiwindow)
#add_example(memory_vma_allocator)
add_cpp_example(vao_multibuffer)
#add_cpp_example(shader_inspection)
add_cpp_example(compute)
add_cpp_example(shaders_glsl)
add_cpp_example(pipeline_instancing)
if(SUPPORT_VULKAN_BACKEND)
    add_cpp_example(models_raytracing)
endif()
#add_cpp_example(pipeline_uniforms)
add_cpp_example(models_obj)
#add_cpp_example(models_lights)
add_cpp_example(models_glb)
add_cpp_example(models_forwardkinematics)
#add_cpp_example(pipeline_constants)
add_cpp_example(benchmark_cubes)
#add_cpp_example(benchmark_tilemap)
#add_cpp_example(core_screenrecord)
add_cpp_example(textures_formats)
#add_cpp_example(textures_mipmap)
#add_cpp_example(models_gpu_skinning)
#add_cpp_example(textures_bloom)
#if(SUPPORT_GLSL_PARSER)
#    add_cpp_example(glsl_to_wgsl)
#endif()
#add_cpp_example(textures_storage)
if(SUPPORT_VULKAN_BACKEND)
    add_executable(plain_wgvk "plain_wgvk.c")
    target_link_libraries(plain_wgvk PUBLIC wgvk)
    target_compile_features(plain_wgvk PUBLIC c_std_23)

    add_executable(surface_wgvk "surface_wgvk.c")
    target_link_libraries  (surface_wgvk PUBLIC wgvk glfw)
    target_compile_features(surface_wgvk PUBLIC c_std_23)
endif()