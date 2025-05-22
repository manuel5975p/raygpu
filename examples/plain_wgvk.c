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

void reflectionCallback(enum WGVkReflectionInfoRequestStatus status, const struct WGVKReflectionInfo* reflectionInfo, void* userdata1, void* userdata2){
    printf("called backed\n");
}

int main(){
    volkInitialize();
    
    uint32_t propertyCount;
    
    vkEnumerateInstanceLayerProperties(&propertyCount, NULL);
    VkLayerProperties* props = calloc(propertyCount, sizeof(VkLayerProperties)); 
    vkEnumerateInstanceLayerProperties(&propertyCount, props);

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
        .label = "Vertex Modul"
    };

    WGVKShaderModule vertexModule = wgvkDeviceCreateShaderModule(device, &vertexModuleDesc);
    WGVKReflectionInfoCallbackInfo reflectionCallbackInfo = {
        .callback = reflectionCallback,
        .mode = WGVKCallbackMode_WaitAnyOnly,
        .nextInChain = NULL,
        .userdata1 = NULL,
        .userdata2 = NULL
    };

    WGVKFuture reflectionFuture = wgvkShaderModuleGetReflectionInfo(vertexModule, reflectionCallbackInfo);

    WGVKFutureWaitInfo reflectionWaitInfo = {
        .future = reflectionFuture,
        .completed = 0
    };
    wgvkInstanceWaitAny(instance, 1, &reflectionWaitInfo, 1 << 30);
}