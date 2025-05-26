#include <iostream>

#include <tint_c_api.h>
#include <tint/tint.h>
#include <src/tint/lang/wgsl/reader/reader.h>
#include <src/tint/lang/wgsl/program/program.h>
#include <src/tint/lang/wgsl/ast/identifier.h>
#include <src/tint/lang/wgsl/ast/module.h>
#include <src/tint/lang/wgsl/sem/function.h>
static inline ShaderStageMask toShaderStageMask(tint::ast::PipelineStage pstage){
    switch(pstage){
        case tint::ast::PipelineStage::kVertex:
        return ShaderStageMask_Vertex;
        case tint::ast::PipelineStage::kFragment:
        return ShaderStageMask_Fragment;
        case tint::ast::PipelineStage::kCompute:
        return ShaderStageMask_Compute;
        case tint::ast::PipelineStage::kNone:
        return ShaderStageMask(0);
    }
}
struct VarVisibility {
    const tint::ast::Variable* var;
    ShaderStageMask vibibilty;
} ;

RGAPI WGPUReflectionInfo reflectionInfo_wgsl_sync(WGPUStringView wgslSource){
    
    WGPUReflectionInfo ret{};
    size_t length = (wgslSource.length == WGPU_STRLEN) ? std::strlen(wgslSource.data) : wgslSource.length;
    tint::Source::File file("<not a file>", std::string_view(wgslSource.data, wgslSource.data + length));
    tint::Program prog = tint::wgsl::reader::Parse(&file);
    

    std::unordered_map<std::string, VarVisibility> globalsByName;
    static_assert(sizeof(tint::ast::PipelineStage) <= 2);
    
    if(prog.IsValid()){
        for(auto& gv : prog.AST().GlobalVariables()){
            globalsByName.emplace(gv->name->symbol.Name(), VarVisibility{
                .var = gv,
                .vibibilty = ShaderStageMask(0)
            });
        }
        ret.globalCount = prog.AST().GlobalVariables().Length();
        for(const auto& ep : prog.AST().Functions()){
            const auto& sem = prog.Sem().Get(ep);
            for(auto& refbg : sem->TransitivelyReferencedGlobals()){
                const tint::sem::Variable* asvar = refbg->As<tint::sem::Variable>();
                std::string name = asvar->Declaration()->name->symbol.Name();
                globalsByName[name].vibibilty = ShaderStageMask(globalsByName[name].vibibilty | toShaderStageMask(ep->PipelineStage()));
                
            }
        }
        for(auto& gv : prog.AST().GlobalVariables()){
            globalsByName.emplace(gv->name->symbol.Name(), VarVisibility{
                .var = gv,
                .vibibilty = ShaderStageMask(0)
            });
        }
    }

    else{
        std::cerr << "Compilation error\n";
    }
    return ret;
}
RGAPI void reflectionInfo_wgsl_free(WGPUReflectionInfo* reflectionInfo){

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

