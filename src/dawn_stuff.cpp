#include <webgpu/webgpu.h>
#include "include/dawn/native/DawnNative.h"

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
#endif

extern "C" WGPUChainedStruct* chainDawnStuff() {
    static dawn::native::DawnInstanceDescriptor dawnDesc = {};
    
#if defined(_WIN32)
    // On Windows, add System32 as an additional search path (for vulkan-1.dll etc)
    static char systemPath[MAX_PATH] = {0};
    static const char* searchPaths[] = { systemPath };

    if (systemPath[0] == '\0') {
        UINT len = GetSystemDirectoryA(systemPath, MAX_PATH);
        if (len == 0) {
            strcpy(systemPath, "C:\\Windows\\System32\\");
        } 
        // ensure trailing backslash
        else if (len < MAX_PATH - 1 && systemPath[len - 1] != '\\') {
            systemPath[len] = '\\';
            systemPath[len + 1] = '\0';
        }
    }

    dawnDesc.additionalRuntimeSearchPathsCount = 1;
    dawnDesc.additionalRuntimeSearchPaths = searchPaths;
#endif

#if !defined(NDEBUG)
    dawnDesc.backendValidationLevel = dawn::native::BackendValidationLevel::Full;
#else
    dawnDesc.backendValidationLevel = dawn::native::BackendValidationLevel::Disabled;
#endif

    return (WGPUChainedStruct*)&dawnDesc;
}
