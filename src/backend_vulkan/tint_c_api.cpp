#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include <iostream>

#include <src/tint/lang/wgsl/ast/identifier.h>
#include <src/tint/lang/wgsl/ast/module.h>
#include <src/tint/lang/wgsl/program/program.h>
#include <src/tint/lang/wgsl/reader/reader.h>
#include <src/tint/lang/wgsl/sem/function.h>
#include <tint/tint.h>
#include <tint_c_api.h>
#include <raygpu.h>
#undef TRACELOG

#define TRACELOG(...)
static inline WGPUShaderStage toShaderStageMask(tint::ast::PipelineStage pstage) {
    switch (pstage) {
    case tint::ast::PipelineStage::kVertex:
        return WGPUShaderStage_Vertex;
    case tint::ast::PipelineStage::kFragment:
        return WGPUShaderStage_Fragment;
    case tint::ast::PipelineStage::kCompute:
        return WGPUShaderStage_Compute;
    case tint::ast::PipelineStage::kNone:
    default:
        return WGPUShaderStage(0);
    }
}
struct VarVisibility {
    const tint::ast::Variable *var;
    WGPUShaderStage visibilty;
};

static format_or_sample_type extractFormat(const tint::ast::Identifier *iden) {
    if (auto templiden = iden->As<tint::ast::TemplatedIdentifier>()) {
        for (size_t i = 0; i < templiden->arguments.Length(); i++) {
            if (templiden->arguments[i]->As<tint::ast::IdentifierExpression>() &&
                templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier) {
                auto arg_iden = templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier;
                if (arg_iden->symbol.Name() == "r32float") {
                    return format_r32float;
                } else if (arg_iden->symbol.Name() == "r32uint") {
                    return format_r32uint;
                } else if (arg_iden->symbol.Name() == "rgba8unorm") {
                    return format_rgba8unorm;
                } else if (arg_iden->symbol.Name() == "rgba32float") {
                    return format_rgba32float;
                }
                if (arg_iden->symbol.Name() == "f32") {
                    return sample_f32;
                } else if (arg_iden->symbol.Name() == "u32") {
                    return sample_u32;
                }
                // TODO: support all, there's not that many
            }
        }
    }
    return format_or_sample_type(12123);
}
static inline WGPUTextureFormat toStorageTextureFormat(format_or_sample_type fmt) {
    switch (fmt) {
    case format_or_sample_type::format_r32float:
        return WGPUTextureFormat_R32Float;
    case format_or_sample_type::format_r32uint:
        return WGPUTextureFormat_R32Uint;
    case format_or_sample_type::format_rgba8unorm:
        return WGPUTextureFormat_RGBA8Unorm;
    case format_or_sample_type::format_rgba32float:
        return WGPUTextureFormat_RGBA32Float;
    default:
        rg_unreachable();
    }
    return WGPUTextureFormat_Force32;
}
static inline WGPUReflectionComponentType toWGPUComponentType(tint::inspector::ComponentType ctype){
    switch(ctype){
        case tint::inspector::ComponentType::kF32:return WGPUReflectionComponentType_Float32;
        case tint::inspector::ComponentType::kU32:return WGPUReflectionComponentType_Uint32;
        case tint::inspector::ComponentType::kI32:return WGPUReflectionComponentType_Sint32;
        case tint::inspector::ComponentType::kF16:return WGPUReflectionComponentType_Float16;
        case tint::inspector::ComponentType::kUnknown:return WGPUReflectionComponentType_Invalid;
        default: rg_unreachable();
    }
}
static inline WGPUReflectionCompositionType toWGPUCompositionType(tint::inspector::CompositionType ctype){
    switch(ctype){
        case tint::inspector::CompositionType::kScalar: return WGPUReflectionCompositionType_Scalar;
        case tint::inspector::CompositionType::kVec2: return WGPUReflectionCompositionType_Vec2;
        case tint::inspector::CompositionType::kVec3: return WGPUReflectionCompositionType_Vec3;
        case tint::inspector::CompositionType::kVec4: return WGPUReflectionCompositionType_Vec4;
        case tint::inspector::CompositionType::kUnknown: return WGPUReflectionCompositionType_Invalid;
        default: rg_unreachable();
    }
}


RGAPI WGPUReflectionInfo reflectionInfo_wgsl_sync(WGPUStringView wgslSource) {

    WGPUReflectionInfo ret{};
    size_t length = (wgslSource.length == WGPU_STRLEN) ? std::strlen(wgslSource.data) : wgslSource.length;
    tint::Source::File file("<not a file>", std::string_view(wgslSource.data, wgslSource.data + length));
    tint::Program prog = tint::wgsl::reader::Parse(&file);

    std::unordered_map<std::string, VarVisibility> globalsByName;
    static_assert(sizeof(tint::ast::PipelineStage) <= 2);

    if (prog.IsValid()) {
        for (auto &gv : prog.AST().GlobalVariables()) {
            globalsByName.emplace(gv->name->symbol.Name(), VarVisibility{.var = gv, .visibilty = WGPUShaderStage(0)});
        }

        for (const auto &ep : prog.AST().Functions()) {
            const auto &sem = prog.Sem().Get(ep);
            for (auto &refbg : sem->TransitivelyReferencedGlobals()) {
                const tint::sem::Variable *asvar = refbg->As<tint::sem::Variable>();
                std::string name = asvar->Declaration()->name->symbol.Name();
                globalsByName[name].visibilty =
                    WGPUShaderStage(globalsByName[name].visibilty | toShaderStageMask(ep->PipelineStage()));
            }
        }

        ret.globals = (WGPUGlobalReflectionInfo *)RL_CALLOC(globalsByName.size(), sizeof(WGPUGlobalReflectionInfo));
        uint32_t globalInsertIndex = 0;
        for (const auto &[name, varvis] : globalsByName) {

            WGPUGlobalReflectionInfo insert{};
            auto sem = prog.Sem().Get(varvis.var)->As<tint::sem::GlobalVariable>();

            insert.bindGroup = sem->Attributes().binding_point->group;
            insert.binding = sem->Attributes().binding_point->binding;
            insert.name = WGPUStringView{name.c_str(), name.size()};
            insert.visibility = varvis.visibilty;

            auto iden = varvis.var->type->As<tint::ast::IdentifierExpression>()->identifier;
            if (iden->symbol.Name().starts_with("texture_2d")) {
                format_or_sample_type sampletype = extractFormat(iden);
                assert(sampletype == format_or_sample_type::sample_f32 ||
                       sampletype == format_or_sample_type::sample_u32 && "Invalid format or sample type");
                insert.texture.sampleType =
                    (sampletype == format_or_sample_type::sample_f32) ? WGPUTextureSampleType_Float : WGPUTextureSampleType_Uint;

                if (iden->symbol.Name().starts_with("texture_2d_array")) {
                    insert.texture.viewDimension = WGPUTextureViewDimension_2DArray;
                } else {
                    insert.texture.viewDimension = WGPUTextureViewDimension_2D;
                }
            }
            if (iden->symbol.Name().starts_with("texture_3d")) {
                format_or_sample_type sampletype = extractFormat(iden);
                insert.texture.sampleType =
                    (sampletype == format_or_sample_type::sample_f32) ? WGPUTextureSampleType_Float : WGPUTextureSampleType_Uint;
                insert.texture.viewDimension = WGPUTextureViewDimension_3D;
            }
            if (iden->symbol.Name().starts_with("texture_storage_2d")) {
                format_or_sample_type sampletype = extractFormat(iden);
                insert.storageTexture.format = toStorageTextureFormat(sampletype);

                if (varvis.var->As<tint::ast::Var>()
                        ->declared_access->As<tint::ast::IdentifierExpression>()
                        ->identifier->symbol.NameView()
                        .starts_with("read_write"))
                    insert.storageTexture.access = WGPUStorageTextureAccess_ReadWrite;
                else if (varvis.var->As<tint::ast::Var>()
                             ->declared_access->As<tint::ast::IdentifierExpression>()
                             ->identifier->symbol.NameView()
                             .starts_with("readonly"))
                    insert.storageTexture.access = WGPUStorageTextureAccess_ReadOnly;
                else if (varvis.var->As<tint::ast::Var>()
                             ->declared_access->As<tint::ast::IdentifierExpression>()
                             ->identifier->symbol.NameView()
                             .starts_with("writeonly"))
                    insert.storageTexture.access = WGPUStorageTextureAccess_WriteOnly;
                else {
                    assert(false);
                }

                if (iden->symbol.Name().starts_with("texture_storage_2d_array")) {
                    insert.storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
                } else {
                    insert.storageTexture.viewDimension = WGPUTextureViewDimension_2D;
                }

            } else if (iden->symbol.Name().starts_with("texture_storage_3d")) {
                insert.storageTexture.viewDimension = WGPUTextureViewDimension_3D;
                format_or_sample_type sampletype = extractFormat(iden);


            } 
            else if (iden->symbol.Name().starts_with("sampler")) {
                insert.sampler.type = WGPUSamplerBindingType_Filtering;
            }
            else 
            //if (iden->symbol.NameView().starts_with("array")) 
            {
                if (varvis.var->As<tint::ast::Var>()->declared_address_space->As<tint::ast::IdentifierExpression>()->identifier->symbol.NameView().starts_with("storage")) {
                    if (varvis.var->As<tint::ast::Var>()
                            ->declared_access->As<tint::ast::IdentifierExpression>()
                            ->identifier->symbol.NameView()
                            .starts_with("readonly")) {
                        insert.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                    } else {
                        insert.buffer.type = WGPUBufferBindingType_Storage;
                    }
                } else if (varvis.var->As<tint::ast::Var>()
                               ->declared_address_space->As<tint::ast::IdentifierExpression>()
                               ->identifier->symbol.NameView()
                               .starts_with("uniform")) {
                    insert.buffer.type = WGPUBufferBindingType_Uniform;
                }
                insert.buffer.minBindingSize = sem->Type()->UnwrapPtrOrRef()->Size();
            }
            const_cast<WGPUGlobalReflectionInfo *>(ret.globals)[globalInsertIndex++] = insert;
        }
        ret.globalCount = globalsByName.size();

        tint::inspector::Inspector insp(prog);
        namespace ti = tint::inspector;

        const tint::sem::Info &psem = prog.Sem();
        std::unordered_map<std::string, WGPUReflectionAttribute> inputAttributeMap;
        std::unordered_map<std::string, WGPUReflectionAttribute> outputAttributeMap;
        for (size_t i = 0; i < insp.GetEntryPoints().size(); i++) {
            if (insp.GetEntryPoints()[i].stage == ti::PipelineStage::kVertex) {
                for (size_t j = 0; j < insp.GetEntryPoints()[i].input_variables.size(); j++) {
                    // std::string name = insp.GetEntryPoints()[i].input_variables[j].name;
                    std::string varname = insp.GetEntryPoints()[i].input_variables[j].variable_name;
                    if (!insp.GetEntryPoints()[i].input_variables[j].attributes.location.has_value())
                        continue;
                    uint32_t location = insp.GetEntryPoints()[i].input_variables[j].attributes.location.value();
                    WGPUReflectionAttribute insert{
                        .location = location,
                        .componentType = toWGPUComponentType(insp.GetEntryPoints()[i].input_variables[j].component_type),
                        .compositionType = toWGPUCompositionType(insp.GetEntryPoints()[i].input_variables[j].composition_type)
                    };
                    inputAttributeMap[varname] = insert;
                }
                // std::cout << "\n";
            } else if (insp.GetEntryPoints()[i].stage == ti::PipelineStage::kFragment) {
                const std::vector<ti::StageVariable> vec_outvariables = insp.GetEntryPoints()[i].output_variables;
                for (const auto &outvar : vec_outvariables) {
                    uint32_t dim = ~0;
                    format_or_sample_type type = format_or_sample_type::we_dont_know;
                    WGPUReflectionAttribute insert = {
                        .location = outvar.attributes.location.value_or(LOCATION_NOT_FOUND),
                        .componentType = toWGPUComponentType(outvar.component_type),
                        .compositionType = toWGPUCompositionType(outvar.composition_type)
                    };
                    outputAttributeMap[outvar.variable_name] = insert;
                    //retvalue.attachments.emplace_back(outvar.attributes.location.value_or(LOCATION_NOT_FOUND), type);
                }
            }
        }

    }

    else {
        std::cerr << "Compilation error\n";
    }
    return ret;
}
RGAPI void reflectionInfo_wgsl_free(WGPUReflectionInfo *reflectionInfo) {
    RL_FREE((void*)reflectionInfo->globals);
    RL_FREE((void*)reflectionInfo->inputAttributes);
    RL_FREE((void*)reflectionInfo->outputAttributes);
}
RGAPI tc_SpirvBlob wgslToSpirv(const WGPUShaderSourceWGSL *source) {

    size_t length = (source->code.length == WGPU_STRLEN) ? std::strlen(source->code.data) : source->code.length;
    tint::Source::File file("<not a file>", std::string_view(source->code.data, source->code.data + length));
    tint::Program prog = tint::wgsl::reader::Parse(&file);
    tint::Result<tint::core::ir::Module> maybeModule = tint::wgsl::reader::ProgramToLoweredIR(prog);
    if(maybeModule == tint::Success){
        tint::core::ir::Module module(std::move(maybeModule.Get()));
        tint::spirv::writer::Options options{};
        tint::Result<tint::spirv::writer::Output> spirvMaybe = tint::spirv::writer::Generate(module, options);
        tint::spirv::writer::Output output(spirvMaybe.Get());
        tc_SpirvBlob ret{(output.spirv.size() * sizeof(uint32_t)), (uint32_t *)RL_CALLOC(output.spirv.size(), sizeof(uint32_t))};
        std::copy(output.spirv.begin(), output.spirv.end(), ret.code);
        return ret;
    }
    else{
        std::cerr << "Compilation failed: " << maybeModule.Failure().reason << "\n";
        tc_SpirvBlob ret = {0};
        return ret;
    }
}
