#include <ctre.hpp>
#include <raygpu.h>
#include <vector>
#include <sstream>
#include <tint/tint.h>
//#include <tint/lang/wgsl/reader/reader.h>
//#include <tint/api/tint.h>
#include <tint/lang/wgsl/reader/parser/parser.h>
#include <tint/lang/wgsl/reader/reader.h>
std::unordered_map<std::string, UniformDescriptor> getBindings(const char* shaderSource){
    tint::Initialize();
    tint::Source::File f("path", shaderSource);
    auto result = tint::wgsl::reader::Parse(&f);
    if(result.IsValid()){
        result.Symbols().Foreach([](const tint::Symbol& x){
            std::cout << x.Name() << "\n";
        
        });
        for(auto& glob : result.AST().GlobalVariables()){
            //std::cout << glob->name->symbol.Name() << " at " << glob->attributes[0]->As<tint::ast::GroupAttribute>()->expr->As<tint::ast::IntLiteralExpression>()->value << " : " << glob->attributes[1]->As<tint::ast::BindingAttribute>()->expr->As<tint::ast::IntLiteralExpression>()->value << "\n";
        }
        for(auto& glob : result.AST().Functions()){
            //std::cout << glob->params[0]->name->TypeInfo().base->name << "\n";
        }
        for(auto& str : result.AST().TypeDecls()){
            std::cout << str->name->symbol.Name() << "\n";
            //std::cout << str->As<tint::ast::Struct>()->members.Length() << "\n";
            for(auto& mem : str->As<tint::ast::Struct>()->members){
                std::cout << "  " << mem->name->symbol.Name() << "\n";
                for(auto& att : mem->attributes){
                    if(auto loca = att->As<tint::ast::LocationAttribute>()){
                        if(auto intliteral = loca->expr->As<tint::ast::IntLiteralExpression>()){
                            std::cout << "    location(" << intliteral->value << ")\n";
                        }
                    }
                }
            }
        }
        
    }
    else{
        std::cout << result.Diagnostics() << std::endl;
    }
    //tint::Source source();
    return {};
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