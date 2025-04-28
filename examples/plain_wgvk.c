#include <wgvk.h>
void TraceLog(int logType, const char *text, ...){
    
}

void adapterCallbackFunction(
        enum WGVKRequestAdapterStatus status,
        WGVKAdapter adapter,
        struct WGVKStringView label,
        void* userdata1,
        void* userdata2
    ){

}

int main(){
    WGVKInstanceDescriptor instanceDescriptor = {
        .capabilities = {},
        .nextInChain = NULL
    };
    WGVKInstance instance = wgvkCreateInstance(&instanceDescriptor);

    WGVKRequestAdapterOptions adapterOptions = {0};
    adapterOptions.featureLevel = WGVKFeatureLevel_Core;

    WGVKRequestAdapterCallbackInfo adapterCallback = {0};
    adapterCallback.callback = adapterCallbackFunction;
    

    WGVKFuture aFuture = wgvkInstanceRequestAdapter(instance, &adapterOptions, adapterCallback);
    
}