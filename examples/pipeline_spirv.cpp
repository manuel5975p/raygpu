#include <raygpu.h>
#include <tint/tint.h>
#include <tint/lang/spirv/reader/parser/parser.h>
#include <tint/lang/spirv/reader/reader.h>
#include <tint/lang/glsl/writer/writer.h>
#include <tint/lang/core/type/reference.h>
#include <tint/lang/core/ir/module.h>
uint32_t byteswap_uint32(uint32_t x) {
    return ((x & 0x000000FFU) << 24) |  // Move byte 0 to byte 3
           ((x & 0x0000FF00U) << 8)  |  // Move byte 1 to byte 2
           ((x & 0x00FF0000U) >> 8)  |  // Move byte 2 to byte 1
           ((x & 0xFF000000U) >> 24);   // Move byte 3 to byte 0
}


int main(){
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(800, 800, "SPIR-V Shader");
    //SetTargetFPS(1800);
    size_t dataSize;
    char* wgsldata = LoadFileText("../resources/base.wgsl");
    char* data = (char*)LoadFileData("../resources/simple.spv", &dataSize);
    char* rev =  (char*)malloc(dataSize);
    memcpy(rev, data, dataSize);
    for(uint32_t i = 0;i < dataSize/4;i++){
        //data[i] = rev[dataSize - i - 1];
    }
    for(uint32_t i = 0;i < dataSize/4;i++){
        uint32_t* ptr = (uint32_t*)data;
        uint32_t val = ptr[i];
        val = byteswap_uint32(val);
        ptr[i] = val;
    }
    //tint::spirv::File f("path", data);

    tint::Slice<const uint32_t> sl((uint32_t*)data, dataSize);
    tint::spirv::reader::Options options{};
    auto result = tint::spirv::reader::Parse(sl);
    std::cout << result << "\n";
    return 0;

    wgpu::ShaderModuleSPIRVDescriptor shaderCodeDesc{};
    wgpu::ShaderModuleWGSLDescriptor wgslshaderCodeDesc{};

    wgslshaderCodeDesc.code = wgpu::StringView{wgsldata, std::strlen(wgsldata)};
    shaderCodeDesc.nextInChain = nullptr;
    shaderCodeDesc.sType = wgpu::SType::ShaderSourceSPIRV;
    shaderCodeDesc.code = (uint32_t*)data;
    shaderCodeDesc.codeSize = dataSize;
    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc;
    WGPUStringView strv = STRVIEW("wtf");
    wgpu::ShaderModule sh = GetCXXDevice().CreateErrorShaderModule(&shaderDesc, strv);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        DrawFPS(5, 5);
        EndDrawing();
    }
}