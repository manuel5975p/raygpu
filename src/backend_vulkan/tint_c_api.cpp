#include <iostream>

#include <tint_c_api.h>
#include <tint/tint.h>
#include <src/tint/lang/wgsl/reader/reader.h>
#include <src/tint/lang/wgsl/program/program.h>
#include <src/tint/lang/wgsl/ast/identifier.h>
#include <src/tint/lang/wgsl/ast/module.h>
#include <src/tint/lang/wgsl/sem/function.h>


RGAPI WGPUReflectionInfo reflectionInfo_wgsl_sync(WGPUStringView wgslSource){
    
    WGPUReflectionInfo ret{};
    size_t length = (wgslSource.length == WGPU_STRLEN) ? std::strlen(wgslSource.data) : wgslSource.length;
    tint::Source::File file("<not a file>", std::string_view(wgslSource.data, wgslSource.data + length));
    tint::Program prog = tint::wgsl::reader::Parse(&file);
    
    if(prog.IsValid()){
        for(const auto& ep : prog.AST().Functions()){
            if(ep->IsEntryPoint()){
                std::cout << ep->name->symbol.NameView() << "\n";
            }
        }
    }

    else{
        std::cerr << "Compilation error\n";
    }
    return ret;
}

RGAPI tc_SpirvBlob wgslToSpirv(const WGPUShaderSourceWGSL* source){
    
    size_t length = (source->code.length == WGPU_STRLEN) ? std::strlen(source->code.data) : source->code.length;
    tint::Source::File file("<not a file>", std::string_view(source->code.data, source->code.data + length));
    tint::Program prog = tint::wgsl::reader::Parse(&file);
    tint::Result<tint::core::ir::Module> maybeModule = tint::wgsl::reader::ProgramToLoweredIR(prog);
    tint::core::ir::Module module(std::move(maybeModule.Get()));
    tint::spirv::writer::Options options zeroinit;
    tint::Result<tint::spirv::writer::Output> spirvMaybe = tint::spirv::writer::Generate(module, options);
    tint::spirv::writer::Output output(spirvMaybe.Get());
    tc_SpirvBlob ret{
        (output.spirv.size() * sizeof(uint32_t)),
        (uint32_t*)RL_CALLOC(output.spirv.size(), sizeof(uint32_t))
    };
    std::copy(output.spirv.begin(), output.spirv.end(), ret.code);

    return ret;
}

