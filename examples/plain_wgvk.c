#include <wgvk.h>
#include <external/volk.h>
#include <stdio.h>
#include <external/incbin.h>
#include <tint_c_api.h>

INCBIN(default_vert, "../resources/default.vert.spv");
INCBIN(default_frag, "../resources/default.frag.spv");
INCBIN(compute_wgsl, "../resources/simple_compute.wgsl");
void adapterCallbackFunction(
        enum WGPURequestAdapterStatus status,
        WGPUAdapter adapter,
        struct WGPUStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGPUAdapter*)userdata1) = adapter;
}

void reflectionCallback(WGPUReflectionInfoRequestStatus status, const WGPUReflectionInfo* reflectionInfo, void* userdata1, void* userdata2){
    for(uint32_t i = 0;i < reflectionInfo->globalCount;i++){

        const char* typedesc = NULL;

        if(reflectionInfo->globals[i].buffer.type != WGPUBufferBindingType_BindingNotUsed){
            typedesc = "buffer";
        }
        if(reflectionInfo->globals[i].texture.sampleType != WGPUTextureSampleType_BindingNotUsed){
            assert(typedesc == NULL && "Two entries set");
            typedesc = "texture";
        }
        if(reflectionInfo->globals[i].sampler.type != WGPUSamplerBindingType_BindingNotUsed){
            assert(typedesc == NULL && "Two entries set");
            typedesc = "sampler";
        }
        

        char namebuffer[256] = {0};
        
        const WGPUGlobalReflectionInfo* toBePrinted = reflectionInfo->globals + i;

        memcpy(namebuffer, toBePrinted->name.data, toBePrinted->name.length);
        printf("Name: %s, location: %u, type: %s", namebuffer, toBePrinted->binding, typedesc);
        if(reflectionInfo->globals[i].buffer.type != WGPUBufferBindingType_BindingNotUsed){
            printf(", minBindingSize = %d", (int)toBePrinted->buffer.minBindingSize);
        }
        printf("\n");
    }
}

int main(){
    WGPUInstanceLayerSelection lsel = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_InstanceValidationLayerSelection
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
    
    WGPUInstanceDescriptor instanceDescriptor = {
        .nextInChain = 
        #ifdef NDEBUG
        NULL
        #else
        &lsel.chain
        #endif
        ,
        .capabilities = {0}
    };

    WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);

    WGPURequestAdapterOptions adapterOptions = {0};
    adapterOptions.featureLevel = WGPUFeatureLevel_Core;

    WGPURequestAdapterCallbackInfo adapterCallback = {0};
    adapterCallback.callback = adapterCallbackFunction;
    WGPUAdapter requestedAdapter;
    adapterCallback.userdata1 = (void*)&requestedAdapter;
    

    WGPUFuture aFuture = wgpuInstanceRequestAdapter(instance, &adapterOptions, adapterCallback);
    WGPUFutureWaitInfo winfo = {
        .future = aFuture,
        .completed = 0
    };

    wgpuInstanceWaitAny(instance, 1, &winfo, ~0ull);
    WGPUStringView deviceLabel = {"WGPU Device", sizeof("WGPU Device") - 1};

    WGPUDeviceDescriptor ddesc = {
        .nextInChain = 0,
        .label = deviceLabel,
        .requiredFeatureCount = 0,
        .requiredFeatures = NULL,
        .requiredLimits = NULL,
        .defaultQueue = {0},
        .deviceLostCallbackInfo = {0},
        .uncapturedErrorCallbackInfo = {0},
    };
    
    WGPUDevice device = wgpuAdapterCreateDevice(requestedAdapter, &ddesc);
    
    
    WGPUShaderSourceSPIRV vertexSource = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceSPIRV
        },
        .codeSize = gdefault_vertSize,
        .code = (uint32_t*)gdefault_vertData
    };

    WGPUShaderModuleDescriptor vertexModuleDesc = {
        .nextInChain = &vertexSource.chain,
        .label = {
            .data   = "Vertex Modul",
            .length = sizeof("Vertex Modul"),
        }
    };

    WGPUShaderSourceSPIRV fragmentSource = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceSPIRV
        },
        .codeSize = gdefault_fragSize,
        .code = (uint32_t*)gdefault_fragData
    };
    WGPUShaderModuleDescriptor fragmentModuleDesc = {
        .nextInChain = &fragmentSource.chain,
        .label = {
            .data   = "Fragment Modul",
            .length = sizeof("Fragment Modul"),
        }
    };

    WGPUShaderSourceWGSL computeSource = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceWGSL
        },
        .code = {
            .data = (char*)gcompute_wgslData,
            .length = gcompute_wgslSize,
        }
    };
    WGPUShaderModuleDescriptor computeModuleDesc = {
        .nextInChain = &computeSource.chain,
        .label = {
            .data   = "Compute Modul",
            .length = sizeof("Compute Modul"),
        }
    };


    WGPUShaderModule vertexModule = wgpuDeviceCreateShaderModule(device, &vertexModuleDesc);
    WGPUShaderModule fragmentModule = wgpuDeviceCreateShaderModule(device, &fragmentModuleDesc);
    WGPUShaderModule computeModule = wgpuDeviceCreateShaderModule(device, &computeModuleDesc);
    
    WGPUReflectionInfoCallbackInfo reflectionCallbackInfo = {
        .nextInChain = NULL,
        .mode = WGPUCallbackMode_WaitAnyOnly,
        .callback = reflectionCallback,
        .userdata1 = NULL,
        .userdata2 = NULL
    };

    WGPUFuture vertReflectionFuture = wgpuShaderModuleGetReflectionInfo(vertexModule, reflectionCallbackInfo);
    WGPUFuture fragReflectionFuture = wgpuShaderModuleGetReflectionInfo(fragmentModule, reflectionCallbackInfo);
    WGPUFuture computeReflectionFuture = wgpuShaderModuleGetReflectionInfo(computeModule, reflectionCallbackInfo);

    WGPUFutureWaitInfo reflectionWaitInfo[3] = {
        {
            .future = vertReflectionFuture,
            .completed = 0
        },
        {
            .future =   fragReflectionFuture,
            .completed = 0
        },
        {
            .future =   computeReflectionFuture,
            .completed = 0
        },
    };
    wgpuInstanceWaitAny(instance, 1, reflectionWaitInfo + 2, 1 << 30);
    WGPUComputePipelineDescriptor cplDesc = {
        .nextInChain = NULL,
        .label = STRVIEW("Kopmute paipline"),
        .compute = {
            .constantCount = 0,
            .constants = NULL,
            .entryPoint = STRVIEW("compute_main"),
            .module = computeModule,
            .nextInChain = NULL
        }
    };
    ResourceTypeDescriptor bglEntries[1] = {
        [0] = {
            storage_buffer,
            8,
            0,
            readwrite,
            we_dont_know,
            ShaderStageMask_Compute
        }
    };
    
    WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(device, bglEntries, 1);
    WGPUPipelineLayout pllayout = wgpuDeviceCreatePipelineLayout(device, &(WGPUPipelineLayoutDescriptor){
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &layout,
    });

    cplDesc.layout = pllayout;
    WGPUComputePipeline cpl = wgpuDeviceCreateComputePipeline(device, &cplDesc);
    
    WGPUBuffer stbuf = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
        .size = 64,
        .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_MapWrite
    });
    WGPUBuffer readableBuffer = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
        .size = 64,
        .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead
    });

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    float floatData[16] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    };

    ResourceDescriptor entries[1] = {
        (ResourceDescriptor){
            .binding = 0,
            .buffer = stbuf,
            .size = 64,
        }
    };

    wgpuQueueWriteBuffer(queue, stbuf, 0, floatData, sizeof(floatData));
    WGPUBindGroup group = wgpuDeviceCreateBindGroup(device, &(WGPUBindGroupDescriptor){
        .entries = entries,
        .entryCount = 1,
        .layout = layout
    });

    WGPUCommandEncoder cenc = wgpuDeviceCreateCommandEncoder(device, NULL);
    WGPUComputePassEncoder cpenc = wgpuCommandEncoderBeginComputePass(cenc);
    wgpuComputePassEncoderSetPipeline(cpenc, cpl);
    wgpuComputePassEncoderSetBindGroup(cpenc, 0, group, 0, NULL);
    wgpuComputePassEncoderDispatchWorkgroups(cpenc, 16, 1, 1);
    wgpuComputePassEncoderEnd(cpenc);
    wgpuCommandEncoderCopyBufferToBuffer(cenc, stbuf, 0, readableBuffer, 0, 64);
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(cenc);
    wgpuQueueSubmit(queue, 1, &cmdBuffer);
    float* floatRead = NULL;
    wgpuBufferMap(readableBuffer, WGPUMapMode_Read, 0, 64, (void**)&floatRead);
    for(int i = 0;i < 16;i++){
        printf("output value[%d] = %f\n", i, floatRead[i]);
    }
    
}