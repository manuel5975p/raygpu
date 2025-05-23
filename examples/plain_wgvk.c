#include <wgvk.h>
#include <external/volk.h>
#include <stdio.h>
#include <external/incbin.h>
INCBIN(default_vert, "../resources/default.vert.spv");
INCBIN(default_frag, "../resources/default.frag.spv");
void adapterCallbackFunction(
        enum WGVKRequestAdapterStatus status,
        WGVKAdapter adapter,
        struct WGVKStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGVKAdapter*)userdata1) = adapter;
}

void reflectionCallback(WGVKReflectionInfoRequestStatus status, const WGVKReflectionInfo* reflectionInfo, void* userdata1, void* userdata2){
    for(uint32_t i = 0;i < reflectionInfo->globalCount;i++){

        const char* typedesc = NULL;

        if(reflectionInfo->globals[i].buffer.type != WGVKBufferBindingType_BindingNotUsed){
            typedesc = "buffer";
        }
        if(reflectionInfo->globals[i].texture.sampleType != WGVKTextureSampleType_BindingNotUsed){
            assert(typedesc == NULL && "Two entries set");
            typedesc = "texture";
        }
        if(reflectionInfo->globals[i].sampler.type != WGVKSamplerBindingType_BindingNotUsed){
            assert(typedesc == NULL && "Two entries set");
            typedesc = "sampler";
        }

        char namebuffer[1024] = {0};
        memcpy(namebuffer, reflectionInfo->globals[i].name.data, reflectionInfo->globals[i].name.length);
        printf("Name: %s, location: %u, type: %s\n", reflectionInfo->globals[i].name.data, reflectionInfo->globals[i].binding, typedesc);
    }
}

int main(){
    
    WGVKInstanceLayerSelection lsel = {
        .chain = {
            .next = NULL,
            .sType = WGVKSType_InstanceValidationLayerSelection
        }
    };
    const char* layernames[] = {"VK_LAYER_KHRONOS_validation"};
    lsel.instanceLayers = layernames;
    lsel.instanceLayerCount = 1;
    //for(uint32_t i = 0;i < propertyCount;i++){
    //    if(strstr(props[i].layerName, "validation") != NULL){
    //        lsel.instanceLayers = (const char* const *)(&(props[i].layerName));
    //        break;
    //    }
    //}
    //__builtin_dump_struct(&lsel, printf);
    
    WGVKInstanceDescriptor instanceDescriptor = {
        .capabilities = {0},
        .nextInChain = &lsel.chain
    };

    WGVKInstance instance = wgvkCreateInstance(&instanceDescriptor);

    WGVKRequestAdapterOptions adapterOptions = {0};
    adapterOptions.featureLevel = WGVKFeatureLevel_Core;

    WGVKRequestAdapterCallbackInfo adapterCallback = {0};
    adapterCallback.callback = adapterCallbackFunction;
    WGVKAdapter requestedAdapter;
    adapterCallback.userdata1 = &requestedAdapter;
    

    WGVKFuture aFuture = wgvkInstanceRequestAdapter(instance, &adapterOptions, adapterCallback);
    WGVKFutureWaitInfo winfo = {
        .future = aFuture,
        .completed = 0
    };
    wgvkInstanceWaitAny(instance, 1, &winfo, ~0ull);
    WGVKStringView deviceLabel = {"WGVK Device", sizeof("WGVK Device") - 1};

    WGVKDeviceDescriptor ddesc = {
        .nextInChain = 0,
        .label = deviceLabel,
        .requiredFeatureCount = 0,
        .requiredFeatures = NULL,
        .requiredLimits = NULL,
        .defaultQueue = 0,
        .deviceLostCallbackInfo = 0,
        .uncapturedErrorCallbackInfo = 0,
    };
    
    WGVKDevice device = wgvkAdapterCreateDevice(requestedAdapter, &ddesc);


    WGVKShaderSourceSPIRV vertexSource = {
        .chain = {
            .next = NULL,
            .sType = WGVKSType_ShaderSourceSPIRV
        },
        .codeSize = gdefault_vertSize,
        .code = (uint32_t*)gdefault_vertData
    };

    WGVKShaderModuleDescriptor vertexModuleDesc = {
        .nextInChain = &vertexSource.chain,
        .label = {
            .data   = "Vertex Modul",
            .length = sizeof("Vertex Modul"),
        }
    };

    WGVKShaderSourceSPIRV fragmentSource = {
        .chain = {
            .next = NULL,
            .sType = WGVKSType_ShaderSourceSPIRV
        },
        .codeSize = gdefault_fragSize,
        .code = (uint32_t*)gdefault_fragData
    };
    WGVKShaderModuleDescriptor fragmentModuleDesc = {
        .nextInChain = &fragmentSource.chain,
        .label = {
            .data   = "Fragment Modul",
            .length = sizeof("Fragment Modul"),
        }
    };


    WGVKShaderModule vertexModule = wgvkDeviceCreateShaderModule(device, &vertexModuleDesc);
    WGVKShaderModule fragmentModule = wgvkDeviceCreateShaderModule(device, &fragmentModuleDesc);
    
    WGVKReflectionInfoCallbackInfo reflectionCallbackInfo = {
        .callback = reflectionCallback,
        .mode = WGVKCallbackMode_WaitAnyOnly,
        .nextInChain = NULL,
        .userdata1 = NULL,
        .userdata2 = NULL
    };

    WGVKFuture vertReflectionFuture = wgvkShaderModuleGetReflectionInfo(vertexModule, reflectionCallbackInfo);
    WGVKFuture fragReflectionFuture = wgvkShaderModuleGetReflectionInfo(fragmentModule, reflectionCallbackInfo);

    WGVKFutureWaitInfo reflectionWaitInfo[2] = {
        {
            .future = vertReflectionFuture,
            .completed = 0
        },
        {
            .future =   fragReflectionFuture,
            .completed = 0
        },
    };
    wgvkInstanceWaitAny(instance, 2, reflectionWaitInfo, 1 << 30);
}