#include <raygpu.h>
#include <stdlib.h>
#include <string.h>

const char cubemapVS[] = "#version 450\n"
    "layout(location = 0) in vec3 in_position;\n"
    "layout(location = 1) in vec2 in_uv;\n"
    "layout(location = 2) in vec3 in_normal;\n"
    "layout(location = 3) in vec4 in_color;\n"
    "\n"
    "layout(location = 0) out vec3 localPos;\n"
    "\n"
    "layout(binding = 0) uniform Perspective_View {\n"
    "    mat4 pvmatrix;\n"
    "};\n"
    "\n"
    "void main() {\n"
    "    gl_Position = pvmatrix * vec4(in_position, 1.0);\n"
    "    localPos = in_position;\n"
    "}\n";

const char cubemapFS[] = "#version 450\n"
    "layout(location = 0) in vec3 localPos;\n"
    "\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "layout(binding = 1) uniform textureCube cubeTexture;\n"
    "layout(binding = 2) uniform sampler cubeSampler;\n"
    "\n"
    "void main() {\n"
    "    outColor = texture(samplerCube(cubeTexture, cubeSampler), normalize(localPos));\n"
    "}\n";

Camera3D cam;
Mesh cube;
Shader pl;
TextureCubemap cubemap;
DescribedSampler smp;
float angle;

static Image GenCubemapStrip(int faceSize){
    int w = faceSize * 6;
    int h = faceSize;
    Color* pixels = (Color*)RL_CALLOC((size_t)w * h, sizeof(Color));

    Color faceColors[6] = {
        {255,  80,  80, 255},
        { 80, 255,  80, 255},
        { 80,  80, 255, 255},
        {255, 255,  80, 255},
        {255,  80, 255, 255},
        { 80, 255, 255, 255},
    };

    for(int face = 0; face < 6; face++){
        for(int y = 0; y < faceSize; y++){
            for(int x = 0; x < faceSize; x++){
                int px = face * faceSize + x;
                int border = (x == 0 || x == faceSize - 1 || y == 0 || y == faceSize - 1);
                if(border){
                    pixels[y * w + px] = (Color){40, 40, 40, 255};
                } else {
                    pixels[y * w + px] = faceColors[face];
                }
            }
        }
    }

    return (Image){
        .data = pixels,
        .width = w,
        .height = h,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .rowStrideInBytes = (size_t)w * 4,
    };
}

void setup(){
    cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0, 0, 5},
        .target = CLITERAL(Vector3){0, 0, 0},
        .up = CLITERAL(Vector3){0, 1, 0},
        .fovy = 60.0f,
    };

    cube = GenMeshCube(2.f, 2.f, 2.f);
    pl = LoadShaderFromMemory(cubemapVS, cubemapFS);
    PrepareShader(pl, cube.vao);

    Image strip = GenCubemapStrip(64);
    cubemap = LoadTextureCubemap(strip, CUBEMAP_LAYOUT_LINE_HORIZONTAL);
    UnloadImage(strip);

    smp = LoadSampler(TEXTURE_WRAP_CLAMP, TEXTURE_FILTER_BILINEAR);
    SetShaderTexture(pl, GetUniformLocation(pl, "cubeTexture"), cubemap);
    SetShaderSampler(pl, GetUniformLocation(pl, "cubeSampler"), smp);

    angle = 0.0f;
}

void render(){
    angle += GetFrameTime();
    cam.position = (Vector3){sinf(angle) * 5.f, 2.0f, cosf(angle) * 5.f};

    BeginDrawing();
    ClearBackground(CLITERAL(Color){30, 30, 30, 255});
    BeginShaderMode(pl);
    BeginMode3D(cam);
    BindVertexArray(cube.vao);
    DrawArraysIndexed(RL_TRIANGLES, *cube.ibo, 36);
    EndMode3D();
    EndShaderMode();
    DrawFPS(0, 0);
    EndDrawing();
}

int main(void){
    InitProgram((ProgramInfo){
        .windowTitle = "Cubemap Texture",
        .windowWidth = 800,
        .windowHeight = 600,
        .setupFunction = setup,
        .renderFunction = render,
    });
}
