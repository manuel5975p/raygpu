struct VertexInput {
    @location(0) position: vec2f,
    @builtin(vertex_index) vindex: u32
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(f32(in.vindex), f32(in.vindex / 2), 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(1,1,1,1);
}