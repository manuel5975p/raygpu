#include <glslang/Public/ShaderLang.h>
#include <iostream>
#include <string.h>
int main(){
    glslang::TShader shader(EShLanguage::EShLangVertex);/*
    glslang_program_t* prog = glslang_program_create();
    glslang_input_t vinput{};
    vinput.stage = GLSLANG_STAGE_VERTEX;
    vinput.code = R"(#version 330 core
layout(location = 0) in vec3 a_pos;

void main() {
    gl_Position = vec4(a_pos, 1.0);
}    
    )";
    glslang_input_t finput{};
    finput.stage = GLSLANG_STAGE_FRAGMENT;
    finput.code = R"(#version 330 core
out vec4 frag_color;

void main() {
    frag_color = vec4(1.0, 0.0, 0.0d, 1.0); // Red
}
    )";
    //auto vertex = glslang_shader_create(&vinput);
    //auto fragment = glslang_shader_create(&finput);
    //glslang_program_add_shader(prog, vertex);
    //glslang_program_add_shader(prog, fragment);
    glslang_program_add_source_text(prog, GLSLANG_STAGE_VERTEX, vinput.code, strlen(vinput.code));
    glslang_program_add_source_text(prog, GLSLANG_STAGE_FRAGMENT, finput.code, strlen(finput.code));
    std::cout << glslang_program_get_info_log(prog) << "\n";*/
}