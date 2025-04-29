#include <wgvk.h>

void adapterCallbackFunction(
        enum WGVKRequestAdapterStatus status,
        WGVKAdapter adapter,
        struct WGVKStringView label,
        void* userdata1,
        void* userdata2
    ){
    printf("helo %p\n", adapter);
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
    WGVKFutureWaitInfo winfo = {
        .future = aFuture,
        .completed = 0
    };
    wgvkInstanceWaitAny(instance, 1, &winfo, ~0ull);
}