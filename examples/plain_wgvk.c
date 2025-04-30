#include <wgvk.h>
#include <external/volk.h>
void adapterCallbackFunction(
        enum WGVKRequestAdapterStatus status,
        WGVKAdapter adapter,
        struct WGVKStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGVKAdapter*)userdata1) = adapter;
}

int main(){
    uint32_t propertyCount;
    volkInitialize();
    vkEnumerateInstanceLayerProperties(&propertyCount, NULL);
    VkLayerProperties* props = calloc(propertyCount, sizeof(VkLayerProperties)); 
    vkEnumerateInstanceLayerProperties(&propertyCount, props);

    WGVKInstanceLayerSelection lsel = {0};
    for(uint32_t i = 0;i < propertyCount;i++){
        if(strstr(props[i].layerName, "validation") != NULL){
            lsel.instanceLayerCount = 1;
            lsel.instanceLayers = &(props[i].layerName);
            break;
        }
    }
    
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
    
    wgvkAdapterCreateDevice(requestedAdapter, &ddesc);
}