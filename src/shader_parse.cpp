#include <ctre.hpp>
#include <raygpu.h>
#include <vector>
#include <sstream>
std::vector<UniformDescriptor> getBindings(const char* shaderSource){
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

    for (auto match: ctre::search_all<"@group\\(([0-9]+)\\) +@binding\\(([0-9]+)\\) *var(<storage>|<uniform>){0,1} *([a-zA-Z0-9_]+) *: *([^;]+)">(accum)) {
        std::cout << std::string_view{match.get<1>()} << ":" << std::string_view{match.get<2>()} << ":" << std::string_view{match.get<3>()} << ":" << std::string_view{match.get<4>()} << ":" << std::string_view{match.get<5>()} << "\n";
    }
    // ctre::match<".">(Iterator &current, const EndIterator last, const flags &f)
    return {};
}