@group(0) @binding(0) var<storage, read_write> data: array<f32>;

//@group(0) @binding(1) var<uniform> constants: mat4x4<f32>;

@compute @workgroup_size(1, 1, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    data[id.x] = data[id.x] * data[id.x];
}