#include <iostream>

#include <tint_c_api.h>
#include <tint/tint.h>
#include <src/tint/lang/wgsl/reader/reader.h>
//#include <src/tint/lang/wgsl/program/program.h>
#include <src/tint/lang/wgsl/ast/identifier.h>
#include <src/tint/lang/wgsl/ast/module.h>
//#include <src/tint/lang/wgsl/sem/function.h>


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
void kakfunc(){
    
    

}