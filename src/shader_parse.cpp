#include <ctre.hpp>
#include <raygpu.h>
#include <vector>
std::vector<UniformDescriptor> getBindings(const char* shaderSource){
    for (auto match: ctre::search_all<"([0-9]+),?">(shaderSource)) {
        std::cout << std::string_view{match.get<0>()} << "\n";
    }
   // ctre::match<".">(Iterator &current, const EndIterator last, const flags &f)
}