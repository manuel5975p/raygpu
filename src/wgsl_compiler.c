#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tint_c_api.h>
char* replace_extension(const char* filename) {
    const char* ext = ".wgsl";
    const char* new_ext = ".spv";
    size_t len = strlen(filename);
    size_t ext_len = strlen(ext);

    // Check if filename ends with .wgsl
    if (len >= ext_len && strcmp(filename + len - ext_len, ext) == 0) {
        // Allocate new string with same base name, replacing extension
        size_t new_len = len - ext_len + strlen(new_ext);
        char* newname = malloc(new_len + 1);
        if (!newname) return NULL;
        strncpy(newname, filename, len - ext_len);  // Copy base part
        strcpy(newname + len - ext_len, new_ext);   // Append new extension
        return newname;
    } else {
        // Doesn't end with .wgsl, append .spv
        char* newname = malloc(len + strlen(new_ext) + 1);
        if (!newname) return NULL;
        strcpy(newname, filename);
        strcat(newname, new_ext);
        return newname;
    }
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <filename.wgsl>\n", argv[0]);
        return 1;
    }
    const char* filename = argv[1];
    FILE* f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = calloc(1, size + 1);
    size_t read = fread(buffer, 1, size, f);
    buffer[read] = '\0'; //not actually required because source.code will be a WGPUStringView

    WGPUShaderSourceWGSL source = {
        .chain.sType = WGPUSType_ShaderSourceWGSL,
        .code = {
            .data = buffer,
            .length = size
        }
    };
    tc_SpirvBlob spirv = wgslToSpirv(&source);
    char* outname = replace_extension(filename);
    FILE* outputfile = fopen(outname, "w");
    fwrite((char*)spirv.code, 1, spirv.codeSize, outputfile);
    fclose(outputfile);
}
