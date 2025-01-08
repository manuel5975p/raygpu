//#include <ctre.hpp>
#include <raygpu.h>
#include <vector>
#include <sstream>
#if defined(SUPPORT_WGSL_PARSER) && SUPPORT_WGSL_PARSER == 1
#include <tint/tint.h>
#include <tint/api/tint.h>
#include <tint/lang/wgsl/reader/parser/parser.h>
#include <tint/lang/wgsl/reader/reader.h>
#include <tint/lang/core/type/reference.h>
#endif

//#include <tint/lang/glsl/writer/writer.h>
//#include <tint/lang/wgsl/reader/reader.h>

const std::unordered_map<std::string, WGPUVertexFormat> builtins = [](){
    std::unordered_map<std::string, WGPUVertexFormat> map;
    map["vec2u"] = WGPUVertexFormat_Uint32x2;
    map["vec3u"] = WGPUVertexFormat_Uint32x3;
    map["vec4u"] = WGPUVertexFormat_Uint32x4;
    map["vec2f"] = WGPUVertexFormat_Float32x2;
    map["vec3f"] = WGPUVertexFormat_Float32x3;
    map["vec4f"] = WGPUVertexFormat_Float32x4;
    return map;
}();
const std::unordered_map<std::string, std::unordered_map<std::string, WGPUVertexFormat>> builtins_templated = [](){
    std::unordered_map<std::string, std::unordered_map<std::string, WGPUVertexFormat>> map;
    map["vec2"]["f32"] = WGPUVertexFormat_Float32x2;
    map["vec3"]["f32"] = WGPUVertexFormat_Float32x3;
    map["vec4"]["f32"] = WGPUVertexFormat_Float32x4;
    map["vec2"]["u32"] = WGPUVertexFormat_Uint32x2;
    map["vec3"]["u32"] = WGPUVertexFormat_Uint32x3;
    map["vec4"]["u32"] = WGPUVertexFormat_Uint32x4;
    return map;
}();
format_or_sample_type extractFormat(const tint::ast::Identifier* iden){
    if(auto templiden = iden->As<tint::ast::TemplatedIdentifier>()){
        for(size_t i = 0;i < templiden->arguments.Length();i++){
            if(templiden->arguments[i]->As<tint::ast::IdentifierExpression>() && templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier){
                auto arg_iden = templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier;
                if(arg_iden->symbol.Name() == "r32float"){
                    return format_r32float;
                }
                else if(arg_iden->symbol.Name() == "r32uint"){
                    return format_r32uint;
                }
                if(arg_iden->symbol.Name() == "f32"){
                    return sample_f32;
                }
                else if(arg_iden->symbol.Name() == "u32"){
                    return sample_u32;
                }
                //TODO: support all, there's not that many
            }
        }
    }
    TRACELOG(LOG_FATAL, "Shader parse failed");
    return format_or_sample_type(12123);
}
access_type extractAccess(const tint::ast::Identifier* iden){
    if(auto templiden = iden->As<tint::ast::TemplatedIdentifier>()){
        for(size_t i = 0;i < templiden->arguments.Length();i++){
            if(templiden->arguments[i]->As<tint::ast::IdentifierExpression>() && templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier){
                auto arg_iden = templiden->arguments[i]->As<tint::ast::IdentifierExpression>()->identifier;
                if(arg_iden->symbol.Name().starts_with("read_write")){
                    return readwrite;
                }
                else if(arg_iden->symbol.Name().starts_with("write")){
                    return writeonly;
                }
                else if(arg_iden->symbol.Name().starts_with("read")){
                    return readonly;
                }
            }
        }
    }
    TRACELOG(LOG_FATAL, "Shader parse failed");
    return access_type(12123);
}
std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> getAttributes(const char* shaderSource){
    
    std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> ret;
#if defined(SUPPORT_WGSL_PARSER) && SUPPORT_WGSL_PARSER == 1
    tint::Source::File f("path", shaderSource);
    tint::wgsl::reader::Options options{};
    tint::wgsl::AllowedFeatures features;
    features.features.emplace(tint::wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);
    options.allowed_features = features;
    
    tint::Program result = tint::wgsl::reader::Parse(&f, options);
    if(!result.IsValid()){
        TRACELOG(LOG_ERROR, "Could not parse shader:");
        std::cout << result.Diagnostics() << "\n";
        return {};
    }
    tint::inspector::Inspector insp(result);
    namespace ti = tint::inspector;
    const tint::sem::Info& psem = result.Sem();
    for(size_t i = 0;i < insp.GetEntryPoints().size();i++){
        if(insp.GetEntryPoints()[i].stage == tint::inspector::PipelineStage::kVertex){
            for(size_t j = 0;j < insp.GetEntryPoints()[i].input_variables.size();j++){
                //std::string name = insp.GetEntryPoints()[i].input_variables[j].name;
                std::string varname = insp.GetEntryPoints()[i].input_variables[j].variable_name;
                if(!insp.GetEntryPoints()[i].input_variables[j].attributes.location.has_value())
                    continue;
                uint32_t location = insp.GetEntryPoints()[i].input_variables[j].attributes.location.value();
                //std::cout << name << " and " << varname << "\n";
                //std::cout << insp.GetEntryPoints()[i].input_variables[j].attributes.location.value() << ": ";
                //std::cout << (int)insp.GetEntryPoints()[i].input_variables[j]. << ", ";
                if(insp.GetEntryPoints()[i].input_variables[j].component_type == ti::ComponentType::kF32){
                    switch(insp.GetEntryPoints()[i].input_variables[j].composition_type){
                        case ti::CompositionType::kScalar:{
                            ret[varname] = {WGPUVertexFormat_Float32, location};
                        }break;
                        case ti::CompositionType::kVec2:{
                            ret[varname] = {WGPUVertexFormat_Float32x2, location};
                        }break;
                        case ti::CompositionType::kVec3:{
                            ret[varname] = {WGPUVertexFormat_Float32x3, location};
                        }break;
                        case ti::CompositionType::kVec4:{
                            ret[varname] = {WGPUVertexFormat_Float32x4, location};
                        }break;
                        case ti::CompositionType::kUnknown:{
                            TRACELOG(LOG_ERROR, "Unknown composition type");
                        }break;
                    }
                }else if(insp.GetEntryPoints()[i].input_variables[j].component_type == ti::ComponentType::kU32){
                    switch(insp.GetEntryPoints()[i].input_variables[j].composition_type){
                        case ti::CompositionType::kScalar:{
                            ret[varname] = {WGPUVertexFormat_Uint32, location};
                        }break;
                        case ti::CompositionType::kVec2:{
                            ret[varname] = {WGPUVertexFormat_Uint32x2, location};
                        }break;
                        case ti::CompositionType::kVec3:{
                            ret[varname] = {WGPUVertexFormat_Uint32x3, location};
                        }break;
                        case ti::CompositionType::kVec4:{
                            ret[varname] = {WGPUVertexFormat_Uint32x4, location};
                        }break;
                        case ti::CompositionType::kUnknown:{
                            TRACELOG(LOG_ERROR, "Unknown composition type");
                        }break;
                    }
                }
                else if(insp.GetEntryPoints()[i].input_variables[j].component_type == ti::ComponentType::kI32){
                    switch(insp.GetEntryPoints()[i].input_variables[j].composition_type){
                        case ti::CompositionType::kScalar:{
                            ret[varname] = {WGPUVertexFormat_Sint32, location};
                        }break;
                        case ti::CompositionType::kVec2:{
                            ret[varname] = {WGPUVertexFormat_Sint32x2, location};
                        }break;
                        case ti::CompositionType::kVec3:{
                            ret[varname] = {WGPUVertexFormat_Sint32x3, location};
                        }break;
                        case ti::CompositionType::kVec4:{
                            ret[varname] = {WGPUVertexFormat_Sint32x4, location};
                        }break;
                        case ti::CompositionType::kUnknown:{
                            TRACELOG(LOG_ERROR, "Unknown composition type");
                        }break;
                    }
                }else if(insp.GetEntryPoints()[i].input_variables[j].component_type == ti::ComponentType::kF16){
                    switch(insp.GetEntryPoints()[i].input_variables[j].composition_type){
                        case ti::CompositionType::kScalar:{
                            ret[varname] = {WGPUVertexFormat_Float16, location};
                        }break;
                        case ti::CompositionType::kVec2:{
                            ret[varname] = {WGPUVertexFormat_Float16x2, location};
                        }break;
                        case ti::CompositionType::kVec3:{
                            TRACELOG(LOG_ERROR, "That should not be possible: vec3<f16>");
                        }break;
                        case ti::CompositionType::kVec4:{
                            ret[varname] = {WGPUVertexFormat_Float16x4, location};
                        }break;
                        case ti::CompositionType::kUnknown:{
                            TRACELOG(LOG_ERROR, "Unknown composition type");
                        }break;
                    }
                }
                else{
                    TRACELOG(LOG_ERROR, "Unknown component type");
                }
                
                //std::cout << (int)insp.GetEntryPoints()[i].input_variables[j].composition_type << ", ";
            }
            //std::cout << "\n";
        }
    }
#endif
    return ret;
}
std::unordered_map<std::string, UniformDescriptor> getBindings(const char* shaderSource){
    //tint::Initialize();
    std::unordered_map<std::string, UniformDescriptor> ret;
#if defined(SUPPORT_WGSL_PARSER) && SUPPORT_WGSL_PARSER == 1
    tint::Source::File f("path", shaderSource);
    tint::wgsl::reader::Options options{};
    tint::wgsl::AllowedFeatures features;
    features.features.emplace(tint::wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);
    options.allowed_features = features;
    
    tint::Program result = tint::wgsl::reader::Parse(&f, options);
    tint::inspector::Inspector insp(result);
    if(!result.IsValid()){
        TRACELOG(LOG_ERROR, "Could not parse shader:");
        std::cout << result.Diagnostics() << "\n";
        return {};
    }
    const tint::sem::Info& psem = result.Sem();
    for(size_t i = 0;i < result.AST().GlobalVariables().Length();i++){
        auto ast_equivalent = result.AST().GlobalVariables()[i];
        if(ast_equivalent->As<tint::ast::Const>() || ast_equivalent->As<tint::ast::Override>()){
            continue; // Skip constants, only process uniforms
        }
        auto sgvar = psem.Get(result.AST().GlobalVariables()[i])->As<tint::sem::GlobalVariable>();
        UniformDescriptor desc{};
        std::stringstream sstr;
        std::string varname = sgvar->Declaration()->name->symbol.Name();
        sstr << sgvar->Type()->FriendlyName();
        TRACELOG(LOG_DEBUG, "Parsing uniform %s of type %s", varname.c_str(), sstr.str().c_str());
        desc.minBindingSize = sgvar->Type()->As<tint::core::type::Reference>()->UnwrapRef()->Size();
        desc.location = sgvar->Attributes().binding_point->binding;
        auto iden = result.AST().GlobalVariables()[i]->type->As<tint::ast::IdentifierExpression>()->identifier;
        auto& glob = result.AST().GlobalVariables()[i];
        if(iden->symbol.Name().starts_with("texture_2d")){
            desc.type = texture2d;
            desc.fstype = extractFormat(iden);
        }
        if(iden->symbol.Name().starts_with("texture_3d")){
            desc.type = texture3d;
            desc.fstype = extractFormat(iden);
        }
        if(iden->symbol.Name().starts_with("texture_storage_2d")){
            desc.type = storage_texture2d;
            desc.access = extractAccess(iden);
            desc.fstype = extractFormat(iden);
        }
        if(iden->symbol.Name().starts_with("texture_storage_3d")){
            desc.type = storage_texture3d;
            desc.access = extractAccess(iden);
            desc.fstype = extractFormat(iden);
        }
        else if(iden->symbol.Name().starts_with("sampler")){
            desc.type = sampler;
        }
        else{
            if(glob->As<tint::ast::Var>()->declared_address_space){
                if(auto addrspace = glob->As<tint::ast::Var>()->declared_address_space->As<tint::ast::IdentifierExpression>()){
                    if(addrspace->identifier->symbol.Name() == "uniform"){
                        desc.type = uniform_buffer;
                    }
                    else if(addrspace->identifier->symbol.Name() == "storage"){
                        if(glob->As<tint::ast::Var>()->declared_access){
                            if(auto accex = glob->As<tint::ast::Var>()->declared_access->As<tint::ast::IdentifierExpression>()){
                                std::string access_modifier = accex->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name();
                                if(access_modifier == "read"){
                                    desc.access = readonly;
                                }
                                else if (access_modifier == "read_write"){
                                    desc.access = readwrite;
                                }
                            }
                        }
                        else{
                            desc.type = storage_buffer;
                        }
                    }
                }
            }
        }       
        
        ret[sgvar->Declaration()->name->symbol.Name()] = desc;
        //std::cout << sgvar->Attributes().binding_point.value() << " ";
        //std::cout << sgvar->Type()->As<tint::core::type::Reference>()->UnwrapRef()->Size() << "\n";
    }
    //auto mod = tint::wgsl::reader::ProgramToLoweredIR(result);
    //std::cout << mod << std::endl;
    //auto glslresult = tint::glsl::writer::Generate(mod.Get(), tint::glsl::writer::Options{}, std::string("vs_main"));
    //std::cout << glslresult << std::endl;
    //std::cout << glslresult.Get().glsl << "\n";
#endif
    return ret;
    /*std::unordered_map<std::string, std::unordered_map<uint32_t, WGPUVertexFormat>> declaredTypesAndLocations;
    if(result.IsValid()){

        for(auto& str : result.AST().TypeDecls()){
            std::string structname =  str->name->symbol.Name();
            //std::cout << structname << "\n";
            if(auto structDecl = str->As<tint::ast::Struct>()){
                for(auto& mem : structDecl->members){
                    //std::cout << mem->type->TypeInfo().name << "\n";
                    //std::cout << "  " << mem->type->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name() << "\n";
                    
                    //bool hasLocationAttribute = false;
                    for(auto& att : mem->attributes){
                        if(auto loca = att->As<tint::ast::LocationAttribute>()){
                            //hasLocationAttribute = true;
                            if(auto intliteral = loca->expr->As<tint::ast::IntLiteralExpression>()){
                                uint32_t v = intliteral->value;
                                if(intliteral->value < 0)
                                    TRACELOG(LOG_WARNING, "Negative attribute location detected");
                                if(auto templ = mem->type->As<tint::ast::IdentifierExpression>()->identifier->As<tint::ast::TemplatedIdentifier>()){
                                    std::string arg0 = templ->arguments[0]->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name();
                                      declaredTypesAndLocations[structname][v] = builtins_templated.at(mem->type->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name()).at(arg0);
                                }
                                else{
                                    declaredTypesAndLocations[structname][v] = builtins.at(mem->type->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name());
                                }
                            }
                        }
                    }
                }
            }
        }
        for(auto& glob : result.AST().GlobalVariables()){
            UniformDescriptor desc{};
            desc.minBindingSize = 4;
            auto iden = glob->type->As<tint::ast::IdentifierExpression>()->identifier;
            if(auto templ_iden = iden->As<tint::ast::TemplatedIdentifier>()){
                for(auto arg : templ_iden->arguments){
                    //std::cout << arg->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name() << "\n";
                }
            }
            uint32_t bindgroup = 0;
            uint32_t bindpoint = 0;
            for(uint32_t i = 0;i < glob->attributes.Length();i++){
                if(auto gattrib = glob->attributes[i]->As<tint::ast::GroupAttribute>()){
                    bindgroup = gattrib->expr->As<tint::ast::IntLiteralExpression>()->value;
                }
                else if(auto battrib = glob->attributes[i]->As<tint::ast::BindingAttribute>()){
                    bindpoint = battrib->expr->As<tint::ast::IntLiteralExpression>()->value;
                }
            }
            if(bindgroup != 0){
                TRACELOG(LOG_WARNING, "Detected @group attribute != 0: %u", (unsigned)bindgroup);
            }
            

            if(iden->symbol.Name().starts_with("texture_2d")){
                desc.type = texture2d;
            }
            else if(iden->symbol.Name().starts_with("sampler")){
                desc.type = sampler;
            }
            else{
                
                
                if(auto addrspace = glob->As<tint::ast::Var>()->declared_address_space->As<tint::ast::IdentifierExpression>()){
                    if(addrspace->identifier->symbol.Name() == "uniform"){
                        desc.type = uniform_buffer;
                    }
                    else if(addrspace->identifier->symbol.Name() == "storage"){
                        
                        if(auto accex = glob->As<tint::ast::Var>()->declared_access->As<tint::ast::IdentifierExpression>()){
                            std::string access_modifier = accex->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name();
                            if(access_modifier == "read"){
                                desc.type = storage_buffer;
                            }
                            else if (access_modifier == "read_write"){
                                desc.type = storage_write_buffer;
                            }
                        }
                        else{
                            desc.type = storage_buffer;
                        }
                    }
                }
                
                
                //std::cout << ">" << std::endl;
                //for(auto attr : glob->TypeInfo().name){
                //    std::cout <<  << "\n";
                //}
                //desc.
            }
            desc.location = bindpoint;
            ret[glob->name->symbol.Name()] = desc;
            //std::cout <<  << "\n";
            //std::cout << glob->type->As<tint::ast::IdentifierExpression>()->identifier->TypeInfo().name << "\n";
            //std::cout << glob->name->symbol.Name() << " at " << glob->attributes[0]->As<tint::ast::GroupAttribute>()->expr->As<tint::ast::IntLiteralExpression>()->value << " : " << glob->attributes[1]->As<tint::ast::BindingAttribute>()->expr->As<tint::ast::IntLiteralExpression>()->value << "\n";
        }
        
        for(auto& glob : result.AST().Functions()){
            //std::cout << "fn " << glob->params[0]->name->TypeInfo().base->name << "\n";
            //std::cout << "fn " << glob->name->symbol.Name() << "\n";
        }
        
        
    }
    else{
        std::cout << result.Diagnostics() << std::endl;
    }
    //tint::Source source();
    return ret;*/
}
/*std::unordered_map<std::string, UniformDescriptor> getBindings(const char* shaderSource){
    std::istringstream istr(shaderSource);
    std::string accum{};
    std::string line;
    while(std::getline(istr, line)){
        size_t n = line.find("//");
        if(n != std::string::npos){
            line.resize(n);
        }
        accum += line;
    }
    std::vector<UniformDescriptor> ret;
    for (auto match : ctre::search_all<"@group\\(([0-9]+)\\) +@binding\\(([0-9]+)\\) *var(< *storage[^>]*>|< *uniform *>){0,1} *([a-zA-Z0-9_]+) *: *([^;]+)">(accum)) {
        UniformDescriptor val;
        val.minBindingSize = 4;
        std::string_view storagetype{match.get<3>()};
        std::string_view type{match.get<5>()};
        if(!storagetype.empty()){
            std::cout << storagetype << "\n";
        }
        if(storagetype.empty()){
            if(type.starts_with("texture_2d")){
                val.type = texture2d;
            }
            else{
                val.type = sampler;
            }
        }
        else{
            if(auto matsch = ctre::search<"< *([a-z]+) *(, *([a-z_]+)){0,1} *>">(storagetype)){
                if(!matsch.get<3>()){
                    val.type = storage_buffer;
                }
                else if(matsch.get<3>() == "read_write"){
                    val.type = storage_write_buffer;
                }
                else if(matsch.get<3>() == "read"){
                    val.type = storage_buffer;
                }
                else{
                    std::string prv(match.get<0>());
                    TRACELOG(LOG_WARNING, "Could not parse entry %s", prv.c_str());
                }
            }
            else{
                std::string prv(match.get<0>());
                TRACELOG(LOG_WARNING, "Could not parse entry %s", prv.c_str());
            }
            
        }
        ret.push_back(val);
    }
    return ret;
}
std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> getAttributes(const char* shaderSource){
    std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> retval;
    std::istringstream istr(shaderSource);
    std::string accum{};
    std::string line;
    while(std::getline(istr, line)){
        size_t n = line.find("//");
        if(n != std::string::npos){
            line.resize(n);
        }
        accum += line;
    }
    //std::cout << accum << "\n";
    if(auto match = ctre::search<"struct +VertexInput *\\{.*?\\}">(accum)) {
        std::string matsch(match);
        for(auto attr : ctre::search_all<"@location\\(([0-9]+)\\) *([a-zA-Z_]+) *: *([^,]+),">(matsch)){
            //std::cout << attr.get<1>() << ',';
            //std::cout << attr.get<2>() << ',';
            //std::cout << attr.get<3>() << '\n';
        }
    }
    else{
        TRACELOG(LOG_WARNING, "Shader code does not contain struct VertexInput, can't parse it");
    }
    return retval;
}*/