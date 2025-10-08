// begin file src/InitWindow.c
#define Font rlFont
#include <raygpu.h>
#undef Font
#include <c_fs_utils.h>
#include <internals.h>
const char shaderSource[] = "struct VertexInput {\n"
"    @location(0) position: vec3f,\n"
"    @location(1) uv: vec2f,\n"
"    @location(2) normal: vec3f,\n"
"    @location(3) color: vec4f,\n"
"};\n"
"\n"
"struct VertexOutput {\n"
"    @builtin(position) position: vec4f,\n"
"    @location(0) uv: vec2f,\n"
"    @location(1) color: vec4f,\n"
"};\n"
"\n"
"struct LightBuffer {\n"
"    count: u32,\n"
"    positions: array<vec3f>\n"
"};\n"
"\n"
"@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;\n"
"@group(0) @binding(1) var texture0: texture_2d<f32>;\n"
"@group(0) @binding(2) var texSampler: sampler;\n"
"@group(0) @binding(3) var<storage, read> modelMatrix: array<mat4x4f>;\n"
//"@group(0) @binding(4) var<storage> lights: LightBuffer;\n"
//"@group(0) @binding(5) var<storage> lights2: LightBuffer;\n"
"\n"
//"Can be omitted\n"
//"@group(0) @binding(3) var<storage> storig: array<vec4f>;\n"
"\n"
"\n"
"@vertex\n"
"fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {\n"
"    var out: VertexOutput;\n"
"    out.position = Perspective_View * \n"
"                   modelMatrix[instanceIdx] *\n"
"    vec4f(in.position.xyz, 1.0f);\n"
"    out.color = in.color;\n"
"    out.uv = in.uv;\n"
"    return out;\n"
"}\n"
"\n"
"@fragment\n"
"fn fs_main(in: VertexOutput) -> @location(0) vec4f {\n"
"    return textureSample(texture0, texSampler, in.uv).rgba * in.color;\n"
"}\n";

const char vertexSourceGLSL[] = "#version 450\n"
"\n"
// " Input attributes.\n"
"layout(location = 0) in vec3 in_position;  // position\n"
"layout(location = 1) in vec2 in_uv;        // texture coordinates\n"
"layout(location = 2) in vec3 in_normal;    // normal (unused)\n"
"layout(location = 3) in vec4 in_color;     // vertex color\n"
"\n"
// " Outputs to fragment shader.\n"
"layout(location = 0) out vec2 frag_uv;\n"
"layout(location = 1) out vec4 frag_color;\n"
"layout(location = 2) out vec3 out_normal;\n"
// " Uniform block for Perspective_View matrix (binding = 0).\n"
"layout(binding = 0) uniform Perspective_View {\n"
"    mat4 pvmatrix;\n"
"};\n"
"\n"
// " Storage buffer for model matrices (binding = 3).\n"
// " Note: 'buffer' qualifier makes it a shader storage buffer.\n"
"layout(binding = 3) readonly buffer modelMatrix {\n"
"    mat4 modelMatrices[];  // Array of model matrices.\n"
"};\n"
"\n"
"void main() {\n"
"    gl_PointSize = 1.0f;\n"
"    // Compute transformed position using instance-specific model matrix.\n"
"    gl_Position = pvmatrix * modelMatrices[gl_InstanceIndex] * vec4(in_position, 1.0);\n"
"    frag_uv = in_uv;\n"
"    frag_color = in_color;\n"
"    out_normal = in_normal;\n"
"}\n";

const char fragmentSourceGLSL[] = "#version 450\n"
"layout(location = 0) in vec2 frag_uv;\n"
"layout(location = 1) in vec4 frag_color;\n"
"layout(location = 0) out vec4 outColor;\n"
"layout(binding = 1) uniform texture2D texture0;  // Texture (binding = 1)\n"
"layout(binding = 2) uniform sampler texSampler;    // Sampler (binding = 2)\n"
"\n"
"void main() {\n"
"    // Sample the texture using the combined sampler.\n"
"    vec4 texColor = texture(sampler2D(texture0, texSampler), frag_uv);\n"
"    outColor = texColor * frag_color;\n"
"}\n";

const uint32_t default_vert_spv_data[] = {

    0x07230203, 0x00010000, 0x0008000b, 0x0000003e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000e000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000d, 0x00000020, 0x00000027, 0x00000032, 0x00000034, 0x00000036, 0x00000038, 0x0000003b, 0x0000003c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67,
    0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002, 0x435f6c67,
    0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00070005, 0x00000015, 0x73726550, 0x74636570, 0x5f657669, 0x77656956, 0x00000000,
    0x00060006, 0x00000015, 0x00000000, 0x616d7670, 0x78697274, 0x00000000, 0x00030005, 0x00000017, 0x00000000, 0x00050005, 0x0000001c, 0x65646f6d, 0x74614d6c, 0x00786972, 0x00070006, 0x0000001c, 0x00000000, 0x65646f6d, 0x74614d6c, 0x65636972,
    0x00000073, 0x00030005, 0x0000001e, 0x00000000, 0x00070005, 0x00000020, 0x495f6c67, 0x6174736e, 0x4965636e, 0x7865646e, 0x00000000, 0x00050005, 0x00000027, 0x705f6e69, 0x7469736f, 0x006e6f69, 0x00040005, 0x00000032, 0x67617266, 0x0076755f,
    0x00040005, 0x00000034, 0x755f6e69, 0x00000076, 0x00050005, 0x00000036, 0x67617266, 0x6c6f635f, 0x0000726f, 0x00050005, 0x00000038, 0x635f6e69, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000003b, 0x5f74756f, 0x6d726f6e, 0x00006c61, 0x00050005,
    0x0000003c, 0x6e5f6e69, 0x616d726f, 0x0000006c, 0x00030047, 0x0000000b, 0x00000002, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002,
    0x0000000b, 0x00000003, 0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x00000015, 0x00000002, 0x00040048, 0x00000015, 0x00000000, 0x00000005, 0x00050048, 0x00000015, 0x00000000, 0x00000007, 0x00000010, 0x00050048,
    0x00000015, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000017, 0x00000021, 0x00000000, 0x00040047, 0x00000017, 0x00000022, 0x00000000, 0x00040047, 0x0000001b, 0x00000006, 0x00000040, 0x00030047, 0x0000001c, 0x00000003, 0x00040048,
    0x0000001c, 0x00000000, 0x00000005, 0x00050048, 0x0000001c, 0x00000000, 0x00000007, 0x00000010, 0x00040048, 0x0000001c, 0x00000000, 0x00000018, 0x00050048, 0x0000001c, 0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x0000001e, 0x00000018,
    0x00040047, 0x0000001e, 0x00000021, 0x00000003, 0x00040047, 0x0000001e, 0x00000022, 0x00000000, 0x00040047, 0x00000020, 0x0000000b, 0x0000002b, 0x00040047, 0x00000027, 0x0000001e, 0x00000000, 0x00040047, 0x00000032, 0x0000001e, 0x00000000,
    0x00040047, 0x00000034, 0x0000001e, 0x00000001, 0x00040047, 0x00000036, 0x0000001e, 0x00000001, 0x00040047, 0x00000038, 0x0000001e, 0x00000003, 0x00040047, 0x0000003b, 0x0000001e, 0x00000002, 0x00040047, 0x0000003c, 0x0000001e, 0x00000002,
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001,
    0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e,
    0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000001, 0x0004002b, 0x00000006, 0x00000010, 0x3f800000, 0x00040020, 0x00000011, 0x00000003, 0x00000006, 0x0004002b, 0x0000000e, 0x00000013, 0x00000000, 0x00040018, 0x00000014,
    0x00000007, 0x00000004, 0x0003001e, 0x00000015, 0x00000014, 0x00040020, 0x00000016, 0x00000002, 0x00000015, 0x0004003b, 0x00000016, 0x00000017, 0x00000002, 0x00040020, 0x00000018, 0x00000002, 0x00000014, 0x0003001d, 0x0000001b, 0x00000014,
    0x0003001e, 0x0000001c, 0x0000001b, 0x00040020, 0x0000001d, 0x00000002, 0x0000001c, 0x0004003b, 0x0000001d, 0x0000001e, 0x00000002, 0x00040020, 0x0000001f, 0x00000001, 0x0000000e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000001, 0x00040017,
    0x00000025, 0x00000006, 0x00000003, 0x00040020, 0x00000026, 0x00000001, 0x00000025, 0x0004003b, 0x00000026, 0x00000027, 0x00000001, 0x00040020, 0x0000002e, 0x00000003, 0x00000007, 0x00040017, 0x00000030, 0x00000006, 0x00000002, 0x00040020,
    0x00000031, 0x00000003, 0x00000030, 0x0004003b, 0x00000031, 0x00000032, 0x00000003, 0x00040020, 0x00000033, 0x00000001, 0x00000030, 0x0004003b, 0x00000033, 0x00000034, 0x00000001, 0x0004003b, 0x0000002e, 0x00000036, 0x00000003, 0x00040020,
    0x00000037, 0x00000001, 0x00000007, 0x0004003b, 0x00000037, 0x00000038, 0x00000001, 0x00040020, 0x0000003a, 0x00000003, 0x00000025, 0x0004003b, 0x0000003a, 0x0000003b, 0x00000003, 0x0004003b, 0x00000026, 0x0000003c, 0x00000001, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000011, 0x00000012, 0x0000000d, 0x0000000f, 0x0003003e, 0x00000012, 0x00000010, 0x00050041, 0x00000018, 0x00000019, 0x00000017, 0x00000013, 0x0004003d,
    0x00000014, 0x0000001a, 0x00000019, 0x0004003d, 0x0000000e, 0x00000021, 0x00000020, 0x00060041, 0x00000018, 0x00000022, 0x0000001e, 0x00000013, 0x00000021, 0x0004003d, 0x00000014, 0x00000023, 0x00000022, 0x00050092, 0x00000014, 0x00000024,
    0x0000001a, 0x00000023, 0x0004003d, 0x00000025, 0x00000028, 0x00000027, 0x00050051, 0x00000006, 0x00000029, 0x00000028, 0x00000000, 0x00050051, 0x00000006, 0x0000002a, 0x00000028, 0x00000001, 0x00050051, 0x00000006, 0x0000002b, 0x00000028,
    0x00000002, 0x00070050, 0x00000007, 0x0000002c, 0x00000029, 0x0000002a, 0x0000002b, 0x00000010, 0x00050091, 0x00000007, 0x0000002d, 0x00000024, 0x0000002c, 0x00050041, 0x0000002e, 0x0000002f, 0x0000000d, 0x00000013, 0x0003003e, 0x0000002f,
    0x0000002d, 0x0004003d, 0x00000030, 0x00000035, 0x00000034, 0x0003003e, 0x00000032, 0x00000035, 0x0004003d, 0x00000007, 0x00000039, 0x00000038, 0x0003003e, 0x00000036, 0x00000039, 0x0004003d, 0x00000025, 0x0000003d, 0x0000003c, 0x0003003e,
    0x0000003b, 0x0000003d, 0x000100fd, 0x00010038
};
const size_t default_vert_spv_data_len = 2096;
const uint32_t default_frag_spv_data[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000020, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000016, 0x0000001a, 0x0000001d, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065, 0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x43786574, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000c, 0x74786574, 0x30657275, 0x00000000, 0x00050005, 0x00000010, 0x53786574, 0x6c706d61, 0x00007265, 0x00040005, 0x00000016,
    0x67617266, 0x0076755f, 0x00050005, 0x0000001a, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000001d, 0x67617266, 0x6c6f635f, 0x0000726f, 0x00040047, 0x0000000c, 0x00000021, 0x00000001, 0x00040047, 0x0000000c, 0x00000022, 0x00000000,
    0x00040047, 0x00000010, 0x00000021, 0x00000002, 0x00040047, 0x00000010, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x0000001e, 0x00000000, 0x00040047, 0x0000001a, 0x0000001e, 0x00000000, 0x00040047, 0x0000001d, 0x0000001e, 0x00000001,
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x00090019, 0x0000000a, 0x00000006, 0x00000001,
    0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00040020, 0x0000000b, 0x00000000, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000000, 0x0002001a, 0x0000000e, 0x00040020, 0x0000000f, 0x00000000, 0x0000000e, 0x0004003b,
    0x0000000f, 0x00000010, 0x00000000, 0x0003001b, 0x00000012, 0x0000000a, 0x00040017, 0x00000014, 0x00000006, 0x00000002, 0x00040020, 0x00000015, 0x00000001, 0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000001, 0x00040020, 0x00000019,
    0x00000003, 0x00000007, 0x0004003b, 0x00000019, 0x0000001a, 0x00000003, 0x00040020, 0x0000001c, 0x00000001, 0x00000007, 0x0004003b, 0x0000001c, 0x0000001d, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003b, 0x00000008, 0x00000009, 0x00000007, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c, 0x0004003d, 0x0000000e, 0x00000011, 0x00000010, 0x00050056, 0x00000012, 0x00000013, 0x0000000d, 0x00000011, 0x0004003d, 0x00000014,
    0x00000017, 0x00000016, 0x00050057, 0x00000007, 0x00000018, 0x00000013, 0x00000017, 0x0003003e, 0x00000009, 0x00000018, 0x0004003d, 0x00000007, 0x0000001b, 0x00000009, 0x0004003d, 0x00000007, 0x0000001e, 0x0000001d, 0x00050085, 0x00000007,
    0x0000001f, 0x0000001b, 0x0000001e, 0x0003003e, 0x0000001a, 0x0000001f, 0x000100fd, 0x00010038
};
const size_t default_frag_spv_data_len = 912;

struct full_renderstate;
#include <renderstate.h>


void PollEvents(){
    #if SUPPORT_SDL2 != 0
    PollEvents_SDL2();
    #endif
    #if SUPPORT_SDL3 != 0
    PollEvents_SDL3();
    #endif
    #if SUPPORT_GLFW != 0
    PollEvents_GLFW();
    #endif
    #if SUPPORT_RGFW != 0
    PollEvents_RGFW();
    #endif
}
void* GetActiveWindowHandle(){
    if(g_renderstate.activeSubWindow && g_renderstate.activeSubWindow->handle){
        return g_renderstate.activeSubWindow->handle;
    }
    return g_renderstate.window;
}
bool WindowShouldClose(cwoid){
    if(g_renderstate.window == NULL){ //headless
        return false;
    }
    #ifdef MAIN_WINDOW_SDL2
    return g_renderstate.closeFlag;
    #elif defined(MAIN_WINDOW_GLFW)
    return WindowShouldClose_GLFW(g_renderstate.window);
    #elif defined(MAIN_WINDOW_SDL3)
    return g_renderstate.closeFlag;
    #else
    return g_renderstate.closeFlag;
    #endif
}

extern Texture2D texShapes;


void* InitWindowEx_ContinuationPoint(InitContext_Impl _ctx);
RGAPI void* InitWindowEx(InitContext_Impl _ctx);
RGAPI void InitBackend(InitContext_Impl _ctx);
RGAPI void* InitWindowEx(InitContext_Impl _ctx){
    InitContext_Impl* ctx = &_ctx;

    #if FORCE_HEADLESS == 1
    g_renderstate.windowFlags |= FLAG_HEADLESS;
    g_renderstate.frameBufferFormat = PIXELFORMAT_UNCOMPRESSED_B8G8R8A8;
    #endif
    ctx->continuationPoint = InitWindowEx_ContinuationPoint;
    InitBackend(_ctx);
    
    return NULL;
}
void* InitWindowEx_ContinuationPoint(InitContext_Impl _ctx){
    InitContext_Impl* ctx = &_ctx;
    if(g_renderstate.windowFlags & FLAG_STDOUT_TO_FFMPEG){
        if(IsATerminal(stdout)){
            TRACELOG(LOG_ERROR, "Refusing to pipe video output to terminal");
            TRACELOG(LOG_ERROR, "Try <program> | ffmpeg -y -f rawvideo -pix_fmt rgba -s %ux%u -r 60 -i - -vf format=yuv420p out.mp4", ctx->windowWidth, ctx->windowHeight);
            exit(1);
        }
        SetTraceLogFile(stderr);
    }
    TRACELOG(LOG_INFO, "Hello!");
    cfs_path workingDirectory = {0};
    cfs_get_working_directory(&workingDirectory);
    
    
    TRACELOG(LOG_INFO, "Working directory: %s", cfs_path_c_str(&workingDirectory));
    g_renderstate.last_timestamps[0] = (int64_t)NanoTime();
    
    
    g_renderstate.width  = ctx->windowWidth;
    g_renderstate.height = ctx->windowHeight;
    
    g_renderstate.whitePixel = LoadTextureFromImage(GenImageChecker(CLITERAL(Color){255,255,255,255}, CLITERAL(Color){255,255,255,255}, 1, 1, 0));
    texShapes = g_renderstate.whitePixel;
    TraceLog(LOG_INFO, "Loaded whitepixel texture");

    //DescribedShaderModule tShader = LoadShaderModuleFromMemory(shaderSource);
    
    Matrix identity = MatrixIdentity();
    g_renderstate.identityMatrix = GenStorageBuffer(&identity, sizeof(Matrix));

    g_renderstate.grst = (GIFRecordState*)RL_CALLOC(1, 160);



    
    
    //void* window = nullptr;
    if(!(g_renderstate.windowFlags & FLAG_HEADLESS)){
        #if SUPPORT_GLFW == 1 || SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1 || SUPPORT_RGFW == 1
        #ifdef MAIN_WINDOW_GLFW
        SubWindow createdWindow = InitWindow_GLFW((int)ctx->windowWidth, (int)ctx->windowHeight, ctx->windowTitle);
        #elif defined(MAIN_WINDOW_SDL3)
        SubWindow createdWindow = InitWindow_SDL3((int)ctx->windowWidth, (int)ctx->windowHeight, ctx->windowTitle);
        #elif defined(MAIN_WINDOW_RGFW)
        SubWindow createdWindow = InitWindow_RGFW((int)ctx->windowWidth, (int)ctx->windowHeight, ctx->windowTitle);
        #else
        Initialize_SDL2();
        SubWindow createdWindow = InitWindow_SDL2((int)ctx->windowWidth, (int)ctx->windowHeight, ctx->windowTitle);
        #endif
        
        WGPUSurface wSurface = (WGPUSurface)CreateSurfaceForWindow(createdWindow);
        CreatedWindowMap_put(&g_renderstate.createdSubwindows, createdWindow->handle, *createdWindow);
        createdWindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, createdWindow->handle);
        createdWindow->surface = CompleteSurface(wSurface, (int)(ctx->windowWidth * createdWindow->scaleFactor), (int)(ctx->windowHeight * createdWindow->scaleFactor));
              
        g_renderstate.window = (GLFWwindow*)createdWindow->handle;

        g_renderstate.mainWindow = createdWindow;
        #endif
    }else{
        g_renderstate.frameBufferFormat = PIXELFORMAT_UNCOMPRESSED_B8G8R8A8;
        CreatedWindowMap_put(&g_renderstate.createdSubwindows, NULL, CLITERAL(RGWindowImpl){0});
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, NULL)->surface = CreateHeadlessSurface(ctx->windowWidth, ctx->windowHeight, g_renderstate.frameBufferFormat);
    }
    


    const ResourceTypeDescriptor uniforms[4] = {
        {uniform_buffer , 64, 0, readonly, we_dont_know, (RGShaderStage)(RGShaderStage_Vertex | RGShaderStage_Fragment)},
        {texture2d,        0, 1, readonly, sample_f32,   (RGShaderStage)(RGShaderStage_Vertex | RGShaderStage_Fragment)},
        {texture_sampler,  0, 2, readonly, we_dont_know, (RGShaderStage)(RGShaderStage_Vertex | RGShaderStage_Fragment)},
        {storage_buffer , 64, 3, readonly, we_dont_know, (RGShaderStage)(RGShaderStage_Vertex | RGShaderStage_Fragment)}
    };

    const AttributeAndResidence attrs[4] = {
        {CLITERAL(RGVertexAttribute){RGVertexFormat_Float32x3, 0 * sizeof(float), 0}, 0, RGVertexStepMode_Vertex, true},
        {CLITERAL(RGVertexAttribute){RGVertexFormat_Float32x2, 3 * sizeof(float), 1}, 0, RGVertexStepMode_Vertex, true},
        {CLITERAL(RGVertexAttribute){RGVertexFormat_Float32x3, 5 * sizeof(float), 2}, 0, RGVertexStepMode_Vertex, true},
        {CLITERAL(RGVertexAttribute){RGVertexFormat_Float32x4, 8 * sizeof(float), 3}, 0, RGVertexStepMode_Vertex, true},
    };
    
    const RGTextureUsage renderAttachmentUsage = RGTextureUsage_RenderAttachment | RGTextureUsage_TextureBinding | RGTextureUsage_CopySrc;

    Texture2D colorTexture = LoadTextureEx(ctx->windowWidth, ctx->windowHeight, g_renderstate.frameBufferFormat, true);
    //g_wgpustate.mainWindowRenderTarget.texture = colorTexture;
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT)
        g_renderstate.mainWindowRenderTarget.colorMultisample = LoadTexturePro(ctx->windowWidth, ctx->windowHeight, g_renderstate.frameBufferFormat, renderAttachmentUsage, 4, 1);
    Texture2D depthTexture = LoadTexturePro(ctx->windowWidth,
                                  ctx->windowHeight,
                                  PIXELFORMAT_DEPTH_32_FLOAT,
                                  renderAttachmentUsage,
                                  (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                                  1
    );
    g_renderstate.mainWindowRenderTarget.depth = depthTexture;
    
    TRACELOG(LOG_INFO, "Renderstate inited");
    g_renderstate.renderExtentX = ctx->windowWidth;
    g_renderstate.renderExtentY = ctx->windowHeight;
    

    LoadFontDefault();
    for(size_t i = 0;i < 4;i++){
        DescribedBufferVector_push_back(&g_renderstate.smallBufferPool, GenVertexBuffer(NULL, sizeof(vertex) * VERTEX_BUFFER_CACHE_SIZE));
    }

    vboptr = (vertex*)RL_CALLOC(10000, sizeof(vertex));
    vboptr_base = vboptr;
    renderBatchVBO = GenVertexBuffer(NULL, ((size_t)(RENDERBATCH_SIZE) * sizeof(vertex)));
    
    renderBatchVAO = LoadVertexArray();
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 0, RGVertexFormat_Float32x3, 0 * sizeof(float), RGVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 1, RGVertexFormat_Float32x2, 3 * sizeof(float), RGVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 2, RGVertexFormat_Float32x3, 5 * sizeof(float), RGVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 3, RGVertexFormat_Float32x4, 8 * sizeof(float), RGVertexStepMode_Vertex);

    const RGColor opaqueBlack = {
        .r = 0.0,
        .g = 0.0,
        .b = 0.0,
        .a = 1.0
    };

    g_renderstate.renderpass = LoadRenderpassEx(GetDefaultSettings(), false, opaqueBlack, false, 0.0f);
    g_renderstate.clearPass = LoadRenderpassEx(GetDefaultSettings(), true, opaqueBlack, true, 1.0f);

    #if SUPPORT_GLSL_PARSER == 1
    ShaderSources defaultGLSLSource zeroinit;
    defaultGLSLSource.sourceCount = 2;
    defaultGLSLSource.sources[0].data = vertexSourceGLSL;
    defaultGLSLSource.sources[0].sizeInBytes = strlen(vertexSourceGLSL);
    defaultGLSLSource.sources[0].stageMask = RGShaderStage_Vertex;
    defaultGLSLSource.sources[1].data = fragmentSourceGLSL;
    defaultGLSLSource.sources[1].sizeInBytes = strlen(fragmentSourceGLSL);
    defaultGLSLSource.sources[1].stageMask = RGShaderStage_Fragment;
    //g_renderstate.defaultShader = LoadPipelineForVAOEx(defaultGLSLSource, renderBatchVAO, uniforms, sizeof(uniforms) / sizeof(ResourceTypeDescriptor), GetDefaultSettings());
    g_renderstate.defaultShader = LoadShaderFromMemory(vertexSourceGLSL, fragmentSourceGLSL);
    #elif SUPPORT_WGSL_PARSER == 1
    ShaderSources defaultWGSLSource zeroinit;
    defaultWGSLSource.sourceCount = 1;
    defaultWGSLSource.sources[0].data = shaderSource;
    defaultWGSLSource.sources[0].sizeInBytes = strlen(shaderSource);
    defaultWGSLSource.sources[0].stageMask = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    //g_renderstate.defaultShader = LoadPipeline(shaderSource);
    g_renderstate.defaultShader = LoadShaderSingleSource(shaderSource);

    #else
    ShaderSources defaultSPIRVSource zeroinit;
    defaultSPIRVSource.sourceCount = 2;
    defaultSPIRVSource.sources[0].data = default_vert_spv_data;
    defaultSPIRVSource.sources[0].sizeInBytes = default_vert_spv_data_len;
    defaultSPIRVSource.sources[0].stageMask = WGPUShaderStage_Vertex;
    defaultSPIRVSource.sources[1].data = default_frag_spv_data;
    defaultSPIRVSource.sources[1].sizeInBytes = default_frag_spv_data_len;
    defaultSPIRVSource.sources[1].stageMask = WGPUShaderStage_Fragment;
    g_renderstate.defaultShader = LoadShaderFromMemorySPIRV(defaultSPIRVSource);
    #endif
    g_renderstate.activeShader = g_renderstate.defaultShader;
    size_t quadCount = 2000;
    g_renderstate.quadindicesCache = GenBufferEx(NULL, quadCount * 6 * sizeof(uint32_t), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);//allocnew(DescribedBuffer);    //WGPUBufferDescriptor vbmdesc{};
    uint32_t* indices = (uint32_t*)RL_CALLOC(6 * quadCount, sizeof(uint32_t));
    for(size_t i = 0;i < quadCount;i++){
        indices[i * 6 + 0] = (i * 4 + 0);
        indices[i * 6 + 1] = (i * 4 + 1);
        indices[i * 6 + 2] = (i * 4 + 3);
        indices[i * 6 + 3] = (i * 4 + 1);
        indices[i * 6 + 4] = (i * 4 + 2);
        indices[i * 6 + 5] = (i * 4 + 3);
    }
    BufferData(g_renderstate.quadindicesCache, indices, 6 * quadCount * sizeof(uint32_t));
    RL_FREE(indices);
    Matrix m = ScreenMatrix(ctx->windowWidth, ctx->windowHeight);
    //static_assert(sizeof(Matrix) == 64, "non 4 byte floats? or what");

    MatrixBufferPair_stack_push(&g_renderstate.matrixStack, CLITERAL(MatrixBufferPair){0});
    SetTexture(1, g_renderstate.whitePixel);
    Matrix iden = MatrixIdentity();
    SetStorageBufferData(3, &iden, 64);
    
    DescribedSampler sampler = LoadSampler(TEXTURE_WRAP_REPEAT, TEXTURE_FILTER_BILINEAR);
    g_renderstate.defaultSampler = sampler;
    SetSampler(2, sampler);
    g_renderstate.init_timestamp = NanoTime();
    g_renderstate.currentSettings = GetDefaultSettings();
    #ifndef __EMSCRIPTEN__
    if((g_renderstate.windowFlags & FLAG_VSYNC_HINT))
        SetTargetFPS(60);
    
    else
        SetTargetFPS(0);
    
    #endif

    if(ctx->finalContinuationPoint){
        ctx->finalContinuationPoint(ctx->setupFunction, ctx->renderFunction);
    }
    return NULL;
}
#if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
    void PLEASE_READ_THE_BUILD_INSTRUCTIONS_FOR_WEB_ON_GITHUB___YOU_CANT_USE_REGULAR_InitWindow_WITHOUT_ASYNCIFY(SetupFunction x, RenderFunction y);
#else
    static void PLEASE_READ_THE_BUILD_INSTRUCTIONS_FOR_WEB_ON_GITHUB___YOU_CANT_USE_REGULAR_InitWindow_WITHOUT_ASYNCIFY(SetupFunction x, RenderFunction y){(void)x;(void)y;}
#endif


RGAPI void InitWindow(int width, int height, const char* title){
    
    InitContext_Impl ctx = {
        .windowTitle = title,
        .windowWidth = width,
        .windowHeight = height,
        .finalContinuationPoint = PLEASE_READ_THE_BUILD_INSTRUCTIONS_FOR_WEB_ON_GITHUB___YOU_CANT_USE_REGULAR_InitWindow_WITHOUT_ASYNCIFY,
    };

    InitWindowEx(ctx);
}

static void InitProgram_continuationPoint(SetupFunction x, RenderFunction y){
    x();
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(y, 0, 0);
    #else
    while(!WindowShouldClose()){
        y();
    }
    #endif
}

RGAPI void InitProgram(ProgramInfo program){
    const char* titleSanitized = program.windowTitle; 
    if(titleSanitized == NULL){
        titleSanitized = "RayGPU Window";
    }
    InitContext_Impl ctx = {
        .windowTitle  = titleSanitized,
        .windowWidth  = program.windowWidth,
        .windowHeight = program.windowHeight,
        .setupFunction = program.setupFunction,
        .renderFunction = program.renderFunction,
        .finalContinuationPoint = InitProgram_continuationPoint,
    };
    InitWindowEx(ctx);
}

RGAPI WGPUSurface CreateSurfaceForWindow(SubWindow window){
    WGPUSurface surfacePtr = NULL;
    window->scaleFactor = 1; // default
    switch (window->type){
        case windowType_sdl3:
        #if SUPPORT_SDL3 == 1
        surfacePtr = CreateSurfaceForWindow_SDL3(window->handle);
        #else
        rg_trap();
        #endif
        break;
        case windowType_glfw:
        #if SUPPORT_GLFW == 1
        surfacePtr = CreateSurfaceForWindow_GLFW(window->handle);
        #else
        rg_trap();
        #endif
        break;
        case windowType_sdl2:
        #if SUPPORT_SDL2 == 1
        surfacePtr = CreateSurfaceForWindow_SDL2(window->handle);
        #else
        rg_trap();
        #endif
        case windowType_rgfw:
        #if SUPPORT_RGFW == 1
        surfacePtr = CreateSurfaceForWindow_RGFW(window->handle);
        #else
        rg_trap();
        #endif
        break;
    }
    TRACELOG(LOG_INFO, "Created surface: %p", surfacePtr);
    return surfacePtr;
}
static inline void CharQueue_Push(window_input_state* s, int codePoint) {
    s->charQueue[s->charQueueTail] = codePoint;
    s->charQueueTail = (s->charQueueTail + 1) % CHARQ_MAX;
    if (s->charQueueCount < CHARQ_MAX) {
        s->charQueueCount++;
    } else {
        s->charQueueHead = (s->charQueueHead + 1) % CHARQ_MAX;
    }
}

void CharCallback(void* window, unsigned int codePoint) {
    CharQueue_Push(&CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state, (int)codePoint);
}

SubWindow OpenSubWindow(int width, int height, const char* title){
    SubWindow createdWindow = NULL;
    #ifdef MAIN_WINDOW_GLFW
    createdWindow = OpenSubWindow_GLFW(width, height, title);
    #elif defined(MAIN_WINDOW_SDL2)
    createdWindow = OpenSubWindow_SDL2(width, height, title);
    #elif defined(MAIN_WINDOW_SDL3)
    createdWindow = OpenSubWindow_SDL3(width, height, title);
    rassert(createdWindow != NULL && createdWindow->handle != NULL, "Returned window can't have null handle");
    #endif
    void* wgpu_or_wgpu_surface = CreateSurfaceForWindow(createdWindow);
    #if SUPPORT_WGPU_BACKEND == 1 || SUPPORT_VULKAN_BACKEND == 1
    WGPUSurface wSurface = (WGPUSurface)wgpu_or_wgpu_surface;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, createdWindow->handle)->surface = CompleteSurface(wSurface, width, height);
    #else
    WGPUSurface vSurface = (WGPUSurface)wgpu_or_wgpu_surface;
    WGPUSurfaceConfiguration config{};
    config.device = g_vulkanstate.device;
    config.width = width;
    config.height = height;
    config.format = WGPUTextureFormat_BGRA8Unorm;
    config.presentMode = WGPUPresentMode_Immediate;
    wgpuSurfaceConfigure(vSurface, &config);
    FullSurface fsurface zeroinit;
    fsurface.surfaceConfig = config;
    fsurface.surface = vSurface;
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT)
        fsurface.renderTarget.colorMultisample = LoadTexturePro(width, height, g_renderstate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 4, 1);
    fsurface.renderTarget.depth = LoadTexturePro(width,
                                  height, 
                                  PIXELFORMAT_DEPTH_32_FLOAT, 
                                  WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                                  (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                                  1
    );
    fsurface.renderTarget.colorAttachmentCount = 1;
    //fsurface.renderTarget.depth = LoadDepthTexture(width, height);
    g_renderstate.createdSubwindows[createdWindow.handle].surface = fsurface;
    #endif
    
    return CreatedWindowMap_get(&g_renderstate.createdSubwindows, createdWindow->handle);
}
void ToggleFullscreenImpl(){
    #ifdef MAIN_WINDOW_GLFW
    ToggleFullscreen_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    ToggleFullscreen_SDL2();
    #elif defined(MAIN_WINDOW_SDL3)
    ToggleFullscreen_SDL3();
    #endif
}
void ToggleFullscreen(){
    g_renderstate.wantsToggleFullscreen = true;
}
Vector2 GetTouchPosition(int index){
    #ifdef MAIN_WINDOW_GLFW
    return CLITERAL(Vector2){0,0};//GetTouchPointCount_GLFW(index);
    #elif defined(MAIN_WINDOW_SDL2)
    return GetTouchPosition_SDL2(index);
    #else
    return CLITERAL(Vector2){0, 0};
    #endif
}
int GetTouchPointCount(cwoid){
    return (int)(CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.activeSubWindow->handle)->input_state.touchPointsCount);
}
int GetMonitorWidth(cwoid){
    #ifdef MAIN_WINDOW_GLFW
    return GetMonitorWidth_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    return GetMonitorWidth_SDL2();
    #elif defined (MAIN_WINDOW_SDL3)
    return GetMonitorWidth_SDL3();
    #endif
    return 0;
}
int GetMonitorHeight(cwoid){
    #ifdef MAIN_WINDOW_GLFW
    return GetMonitorHeight_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    return GetMonitorHeight_SDL2();
    #elif defined (MAIN_WINDOW_SDL3)
    return GetMonitorHeight_SDL3();
    #endif
    return 0;
}
void SetWindowShouldClose(){
    #ifdef MAIN_WINDOW_GLFW
    return SetWindowShouldClose_GLFW(g_renderstate.window);
    #elif defined(MAIN_WINDOW_SDL2)
    g_renderstate.closeFlag = true;
    #endif
}



size_t GetPixelSizeInBytes(PixelFormat format) {
    switch(format){
        case PIXELFORMAT_UNCOMPRESSED_B8G8R8A8:
        case PIXELFORMAT_UNCOMPRESSED_B8G8R8A8_SRGB:
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8_SRGB:
        return 4;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        return 8;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        return 16;

        case GRAYSCALE:
        return 2;
        case RGB8:
        return 3;
        case PIXELFORMAT_DEPTH_24_PLUS:
        case PIXELFORMAT_DEPTH_32_FLOAT:
        return 4;
        case PixelFormat_Force32:
        return 0;
    }
    return 0;
}
#if SUPPORT_GLFW == 1
#include "platform/InitWindow_GLFW.c"
#include "platform/glfw3webgpu.c"
#endif

#if SUPPORT_SDL2 == 1
#include "platform/InitWindow_SDL2.c"
#include "platform/sdl2webgpu.c"
#endif

#if SUPPORT_SDL3 == 1
#include "platform/InitWindow_SDL3.c"
#include "platform/sdl3webgpu.c"
#endif

#if SUPPORT_RGFW == 1
#include "platform/InitWindow_RGFW.c"
#include "platform/rgfwwebgpu.c"
#endif

// end file src/InitWindow.c