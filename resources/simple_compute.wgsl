@group(0) @binding(0) var<storage, read_write> data: array<f32, 5>;

@compute @workgroup_size(32, 1, 1)

fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    data[id.x] = data[id.x] * 2.0f;
}