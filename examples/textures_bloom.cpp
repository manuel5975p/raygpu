#include <raygpu.h>
constexpr char firstPassSource[] = R"(
@group(0) @binding(0) var input: texture_2d<f32>;
@group(0) @binding(1) var output: texture_storage_2d<rgba32float, write>;

@compute @workgroup_size(8,8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let ldval = texture_load(input, id.xy);
}
)";
DescribedComputePipeline* cpl;
int main(){
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(1000, 1000, "Bloom");
    cpl = LoadComputePipeline(firstPassSource);
}