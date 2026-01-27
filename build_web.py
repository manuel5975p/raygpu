import os
import subprocess
import glob
import html

# Configuration
OUTPUT_DIR = "website"
EXAMPLES = [
    "core_window.c",
    "core_shapes.c",
    "benchmark_cubes.c"
]

def main():
    # 1. Create website directory
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        print(f"Created directory: {OUTPUT_DIR}")
    else:
        print(f"Directory {OUTPUT_DIR} already exists.")

    # Read the master shell template
    with open("shell.html", "r") as f:
        shell_template = f.read()

    # 2. Compile examples
    for example in EXAMPLES:
        example_name = os.path.splitext(example)[0]
        output_file = os.path.join(OUTPUT_DIR, f"{example_name}.html")
        example_path = os.path.join("examples", example)
        
        # Check if example file exists
        if not os.path.exists(example_path):
            print(f"Warning: Example file {example_path} not found. Skipping.")
            continue
            
        print(f"Compiling {example}...")
        
        # Read source code
        try:
            with open(example_path, "r") as f:
                source_code = f.read()
            
            # Escape HTML characters to prevent rendering issues
            escaped_code = html.escape(source_code)
            # Also escape emscripten shell special characters
            escaped_code = escaped_code.replace('#', '&#35;')
            escaped_code = escaped_code.replace('{', '&#123;')
            escaped_code = escaped_code.replace('}', '&#125;')
            
            # Create a temporary shell file with the source code injected
            temp_shell_filename = f"shell_{example_name}.html"
            temp_shell_content = shell_template.replace("<!-- SOURCE_CODE_INJECTION -->", escaped_code)
            
            with open(temp_shell_filename, "w") as f:
                f.write(temp_shell_content)
                
            # Construct command using the temporary shell file
            cmd = (
                f"emcc --use-port=emdawnwebgpu {example_path} src/*.c "
                f"-I include -sUSE_SDL=3 -Os -DNDEBUG -sALLOW_MEMORY_GROWTH=1 -sSINGLE_FILE "
                f"--shell-file {temp_shell_filename} "
                f"-o {output_file}"
            )
            
            # Execute compilation
            subprocess.run(cmd, shell=True, check=True)
            print(f"Successfully compiled {example_name}")
            
            # Clean up temporary shell file
            os.remove(temp_shell_filename)
            
        except subprocess.CalledProcessError as e:
            print(f"Error compiling {example}: {e}")
            if os.path.exists(temp_shell_filename):
                 os.remove(temp_shell_filename)
        except Exception as e:
            print(f"An error occurred with {example}: {e}")
            if os.path.exists(temp_shell_filename):
                 os.remove(temp_shell_filename)

    # 3. Create index.html
    generate_index_html()
    print("Build complete.")

def generate_index_html():
    links_html = ""
    for example in EXAMPLES:
        name = os.path.splitext(example)[0]
        display_name = name.replace("_", " ").title()
        links_html += f"""
        <a href="{name}.html" class="example-card">
            <div class="card-content">
                <h2>{display_name}</h2>
                <p>View Example</p>
            </div>
            <div class="card-arrow">→</div>
        </a>
        """

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RayGPU Examples</title>
    <style>
        :root {{
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --accent: #38bdf8;
            --accent-hover: #0ea5e9;
        }}
        
        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}

        #bg-canvas {{
            position: fixed;
            top: 0;
            left: 0;
            width: 100vw;
            height: 100vh;
            z-index: -1;
            background-color: var(--bg-color);
        }}
        
        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: transparent;
            color: var(--text-primary);
            line-height: 1.6;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
        }}

        header {{
            margin-top: 4rem;
            margin-bottom: 3rem;
            text-align: center;
            position: relative;
        }}

        nav {{
            position: absolute;
            top: -3rem;
            right: 1rem;
            display: flex;
            gap: 1rem;
        }}

        .nav-link {{
            color: var(--text-secondary);
            text-decoration: none;
            font-weight: 500;
            transition: all 0.2s;
            display: flex;
            align-items: center;
            gap: 0.5rem;
            padding: 0.5rem 0.75rem;
            border-radius: 6px;
        }}

        .nav-link:hover {{
            color: var(--text-primary);
            background-color: rgba(255, 255, 255, 0.05);
        }}

        .nav-link svg {{
            width: 20px;
            height: 20px;
            fill: currentColor;
        }}

        h1 {{
            font-size: 3rem;
            font-weight: 800;
            background: linear-gradient(135deg, #fff 0%, var(--accent) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 0.5rem;
        }}

        p.subtitle {{
            color: var(--text-secondary);
            font-size: 1.1rem;
        }}

        .grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 1.5rem;
            width: 90%;
            max-width: 1000px;
            padding: 1rem;
        }}

        .example-card {{
            background-color: var(--card-bg);
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 12px;
            padding: 2rem;
            text-decoration: none;
            color: inherit;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            display: flex;
            justify-content: space-between;
            align-items: center;
            position: relative;
            overflow: hidden;
        }}

        .example-card::before {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: linear-gradient(45deg, transparent 0%, rgba(56, 189, 248, 0.05) 100%);
            opacity: 0;
            transition: opacity 0.3s;
        }}

        .example-card:hover {{
            transform: translateY(-5px);
            box-shadow: 0 10px 30px -10px rgba(0, 0, 0, 0.5);
            border-color: rgba(56, 189, 248, 0.3);
        }}

        .example-card:hover::before {{
            opacity: 1;
        }}

        .card-content h2 {{
            font-size: 1.5rem;
            margin-bottom: 0.25rem;
            color: var(--text-primary);
        }}

        .card-content p {{
            color: var(--text-secondary);
            font-size: 0.9rem;
        }}

        .card-arrow {{
            color: var(--accent);
            font-size: 1.5rem;
            opacity: 0;
            transform: translateX(-10px);
            transition: all 0.3s ease;
        }}

        .example-card:hover .card-arrow {{
            opacity: 1;
            transform: translateX(0);
        }}

        footer {{
            margin-top: auto;
            padding: 2rem;
            color: var(--text-secondary);
            font-size: 0.9rem;
        }}
    </style>
</head>
<body>
    <canvas id="bg-canvas"></canvas>
    <header>
        <nav>
            <a href="#" class="nav-link">
                <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M20.317 4.37a19.791 19.791 0 0 0-4.885-1.515.074.074 0 0 0-.079.037c-.21.375-.444.864-.608 1.25a18.27 18.27 0 0 0-5.487 0 12.64 12.64 0 0 0-.617-1.25.077.077 0 0 0-.079-.037A19.736 19.736 0 0 0 3.677 4.37a.07.07 0 0 0-.032.027C.533 9.046-.32 13.58.099 18.057a.082.082 0 0 0 .031.057 19.9 19.9 0 0 0 5.993 3.03.078.078 0 0 0 .084-.028 14.09 14.09 0 0 0 1.226-1.994.076.076 0 0 0-.041-.106 13.107 13.107 0 0 1-1.872-.892.077.077 0 0 1-.008-.128 10.2 10.2 0 0 0 .372-.292.074.074 0 0 1 .077-.01c3.928 1.793 8.18 1.793 12.062 0a.074.074 0 0 1 .078.01c.12.098.246.198.373.292a.077.077 0 0 1-.006.127 12.299 12.299 0 0 1-1.873.892.077.077 0 0 0-.041.107c.36.698.772 1.362 1.225 1.993a.076.076 0 0 0 .084.028 19.839 19.839 0 0 0 6.002-3.03.077.077 0 0 0 .032-.054c.5-5.177-.838-9.674-3.549-13.66a.061.061 0 0 0-.031-.03zM8.02 15.33c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.956-2.419 2.157-2.419 1.21 0 2.176 1.086 2.157 2.419 0 1.334-.956 2.42-2.157 2.42zm7.975 0c-1.183 0-2.157-1.085-2.157-2.419 0-1.333.955-2.419 2.157-2.419 1.21 0 2.176 1.086 2.157 2.419 0 1.334-.946 2.42-2.157 2.42z"/></svg>
                Discord
            </a>
            <a href="#" class="nav-link">
                <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M14 2H6c-1.1 0-1.99.9-1.99 2L4 20c0 1.1.89 2 1.99 2H18c1.1 0 2-.9 2-2V8l-6-6zm2 16H8v-2h8v2zm0-4H8v-2h8v2zm-3-5V3.5L18.5 9H13z"/></svg>
                Docs
            </a>
            <a href="#" class="nav-link">
                <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z"/></svg>
                GitHub
            </a>
        </nav>
        <h1>RayGPU Examples</h1>
        <p class="subtitle">WebAssembly + WebGPU Tech Demos</p>
    </header>

    <main class="grid">
        {links_html}
    </main>

    <footer>
        <p>Powered by raygpu</p>
    </footer>
    <script>
        async function initWebGPU() {{
            if (!navigator.gpu) {{
                console.log("WebGPU not supported");
                return;
            }}

            const canvas = document.getElementById("bg-canvas");
            const adapter = await navigator.gpu.requestAdapter();
            if (!adapter) return;
            let adapterInfo = null;
            
            if (adapter.info) {
                adapterInfo = adapter.info;
            } else if (typeof adapter.requestAdapterInfo === 'function') {
                adapterInfo = await adapter.requestAdapterInfo();
            }
            
            const isCPU = adapterInfo ? (adapterInfo.isFallbackAdapter) : true;
            const NUM_PARTICLES = isCPU ? 200 : 10000;
            
            const device = await adapter.requestDevice();
            const context = canvas.getContext("webgpu");
            const format = "bgra8unorm"; // Firefox requirement often
            
            // Common Structs
            const commonWGSL = `
            struct Particle {{
                pos: vec2<f32>,
                vel: vec2<f32>,
                color: vec4<f32>,
            }}

            struct Params {{
                mouse_pos: vec2<f32>,
                resolution: vec2<f32>,
                time: f32,
                padding: f32,
            }}
            `;

            // Compute Shader: Needs read_write access
            const computeWGSL = commonWGSL + `
            @group(0) @binding(0) var<uniform> params: Params;
            @group(0) @binding(1) var<storage, read_write> particles: array<Particle>;

            @compute @workgroup_size(64)
            fn sim_main(@builtin(global_invocation_id) global_id: vec3<u32>) {{
                let i = global_id.x;
                if (i >= arrayLength(&particles)) {{ return; }}

                var p = particles[i];

                // Screen Adaptation
                let max_res = max(params.resolution.x, params.resolution.y);
                let scale = max_res / 1080.0; 
                let speed_mult = 15.0; 
                
                // Constants scaled for resolution and speed
                let force_strength = 100.0 * speed_mult * scale * scale; // Reduced pull
                let soften = 50.0 * scale;
                let chaos_strength = 20.0 * speed_mult * scale;
                let friction = 0.98;

                let diff = params.mouse_pos - p.pos;
                let dist = length(diff);
                let dir = normalize(diff);

                // Gravity
                p.vel.y += 50.0 * scale;

                // Forces
                // 1. Weak Pull towards mouse
                if (dist < 300.0 * scale) {{
                    p.vel += dir * (force_strength / (dist + soften)) * 0.5;
                }}
                
                // 2. Chaos (Noise)
                let noise_freq = 0.005 / scale; 
                let chaos = vec2<f32>(
                    sin(params.time * 2.0 + p.pos.y * noise_freq),
                    cos(params.time * 1.5 + p.pos.x * noise_freq)
                ) * chaos_strength;

                p.vel += chaos * 0.016;
                p.vel *= friction;
                p.pos += p.vel * 0.016;

                // Bounds
                if (p.pos.x < 0.0) {{ p.pos.x += params.resolution.x; }}
                if (p.pos.x > params.resolution.x) {{ p.pos.x -= params.resolution.x; }}
                
                // Re-enter from top logic
                if (p.pos.y > params.resolution.y) {{ 
                    p.pos.y = 0.0; 
                    // Distribute evenly along top line
                    p.pos.x = (f32(i) / f32(arrayLength(&particles))) * params.resolution.x;
                    p.vel = vec2<f32>(0.0, 0.0 + (50.0 + Math.random() * 50.0) * scale); // Initial logic inside shader is hard, just 0
                    p.vel = vec2<f32>(0.0, 100.0 * scale);
                }}
                // Remove bottom bounce/wrap, relying on top re-enter trigger
                
                // Color based on height (Hot White -> Yellow -> Red)
                let t = clamp(p.pos.y / params.resolution.y, 0.0, 1.0);
                p.color = vec4<f32>(
                    1.0,
                    clamp(2.0 * (1.0 - t), 0.0, 1.0),
                    clamp(1.0 - 2.0 * t, 0.0, 1.0),
                    0.8 // Slightly transparent base
                );

                particles[i] = p;
            }}
            `;

            // Render Shader: Needs read-only access
            const renderWGSL = commonWGSL + `
            @group(0) @binding(0) var<uniform> params: Params;
            @group(0) @binding(1) var<storage, read> particles: array<Particle>;

            struct VertexOutput {{
                @builtin(position) position: vec4<f32>,
                @location(0) color: vec4<f32>,
                @location(1) uv: vec2<f32>,
            }}

            @vertex
            fn vert_main(
                @builtin(vertex_index) vertex_index: u32,
                @builtin(instance_index) instance_index: u32
            ) -> VertexOutput {{
                let p = particles[instance_index];
                
                let size = 3.0 + length(p.vel) * 0.01;
                
                var pos = vec2<f32>(0.0, 0.0);
                var uv = vec2<f32>(0.0, 0.0);

                // Quad vertex generation (6 vertices) - Correct Topology
                // Triangle 1: BL, BR, TL
                if (vertex_index == 0u) {{ pos = vec2<f32>(-1.0, -1.0); uv = vec2<f32>(-1.0, -1.0); }} // BL
                else if (vertex_index == 1u) {{ pos = vec2<f32>( 1.0, -1.0); uv = vec2<f32>( 1.0, -1.0); }} // BR
                else if (vertex_index == 2u) {{ pos = vec2<f32>(-1.0,  1.0); uv = vec2<f32>(-1.0,  1.0); }} // TL
                
                // Triangle 2: BR, TR, TL
                else if (vertex_index == 3u) {{ pos = vec2<f32>( 1.0, -1.0); uv = vec2<f32>( 1.0, -1.0); }} // BR
                else if (vertex_index == 4u) {{ pos = vec2<f32>( 1.0,  1.0); uv = vec2<f32>( 1.0,  1.0); }} // TR
                else if (vertex_index == 5u) {{ pos = vec2<f32>(-1.0,  1.0); uv = vec2<f32>(-1.0,  1.0); }} // TL

                let world_pos = p.pos + pos * size;
                
                let ndc_x = (world_pos.x / params.resolution.x) * 2.0 - 1.0;
                let ndc_y = (1.0 - (world_pos.y / params.resolution.y)) * 2.0 - 1.0;

                var out: VertexOutput;
                out.position = vec4<f32>(ndc_x, ndc_y, 0.0, 1.0);
                out.color = p.color;
                out.uv = uv;
                return out;
            }}

            @fragment
            fn frag_main(@location(0) color: vec4<f32>, @location(1) uv: vec2<f32>) -> @location(0) vec4<f32> {{
                let d = length(uv);
                let alpha = exp(-d * d * 3.0);
                // Premultiplied alpha output
                return vec4<f32>(color.rgb * alpha, alpha);
            }}
            `;

            const computeModule = device.createShaderModule({{ code: computeWGSL }});
            const renderModule = device.createShaderModule({{ code: renderWGSL }});

            // 1. Initial Particle Data
            const particleData = new Float32Array(NUM_PARTICLES * 8);
            for (let i = 0; i < NUM_PARTICLES; i++) {{
                particleData[i*8 + 0] = Math.random() * window.innerWidth;
                particleData[i*8 + 1] = Math.random() * window.innerHeight;
                particleData[i*8 + 2] = (Math.random() - 0.5) * 50;
                particleData[i*8 + 3] = (Math.random() - 0.5) * 50;
                particleData[i*8 + 4] = 1.0;
                particleData[i*8 + 5] = 1.0;
                particleData[i*8 + 6] = 1.0;
                particleData[i*8 + 7] = 1.0;
            }}

            const particleBuffer = device.createBuffer({{
                size: particleData.byteLength,
                usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
            }});
            device.queue.writeBuffer(particleBuffer, 0, particleData);

            // 2. Uniform Buffer
            const uniformBufferSize = 32;
            const uniformBuffer = device.createBuffer({{
                size: uniformBufferSize,
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
            }});

            const uniformData = new Float32Array(8);

            // 3. Pipelines
            const computePipeline = device.createComputePipeline({{
                layout: 'auto',
                compute: {{
                    module: computeModule,
                    entryPoint: 'sim_main',
                }},
            }});

            const renderPipeline = device.createRenderPipeline({{
                layout: 'auto',
                vertex: {{
                    module: renderModule,
                    entryPoint: 'vert_main',
                }},
                fragment: {{
                    module: renderModule,
                    entryPoint: 'frag_main',
                    targets: [{{
                        format: format,
                        blend: {{
                            color: {{ srcFactor: 'one', dstFactor: 'one-minus-src-alpha', operation: 'add' }},
                            alpha: {{ srcFactor: 'one', dstFactor: 'one-minus-src-alpha', operation: 'add' }},
                        }}
                    }}],
                }},
                primitive: {{
                    topology: 'triangle-list',
                }},
            }});

            // Bind Groups
            // We use the same buffer bindings for both compute and render (vertex) 
            // BUT they might need separate bind groups if layouts differ or we can share layouts manually.
            // Using 'auto' layout means we ask the pipeline for the bind group layout.
            // In our shader, both compute and vertex share group(0).
            // So we can create one bind group for both if the layout matches, but 'auto' generates different objects.
            // It's safer to create two bind groups or force a layout.
            // Actually, the shader has uniform and storage in group(0).
            // Compute uses sim_main, vertex uses vert_main. Vertex reads storage.
            // We can try creating one bind group from the compute layout and see if we can use it for render
            // OR just create two identical bind groups.
            
            const bindGroup = device.createBindGroup({{
                layout: computePipeline.getBindGroupLayout(0),
                entries: [
                    {{ binding: 0, resource: {{ buffer: uniformBuffer }} }},
                    {{ binding: 1, resource: {{ buffer: particleBuffer }} }},
                ],
            }});

            // For render pipeline, since we use 'auto', we need to check if the layout is compatible or identical.
            // Usually simpler to just create another one if needed, but here group(0) is defined identically.
            // Note: Vertex shader 'vert_main' DOES access 'particles' (storage) and 'params' (uniform).
            // So it effectively has the same layout requirements for group 0.
            const renderBindGroup = device.createBindGroup({{
                layout: renderPipeline.getBindGroupLayout(0),
                entries: [
                    {{ binding: 0, resource: {{ buffer: uniformBuffer }} }},
                    {{ binding: 1, resource: {{ buffer: particleBuffer }} }},
                ],
            }});
            
            // Mouse tracking
            let mouseX = 0;
            let mouseY = 0;
            window.addEventListener('mousemove', e => {{
                mouseX = e.clientX;
                mouseY = e.clientY;
            }});

            function resize() {{
                const dpr = window.devicePixelRatio || 1;
                canvas.width = window.innerWidth * dpr;
                canvas.height = window.innerHeight * dpr;
                context.configure({{
                    device: device,
                    format: format,
                    alphaMode: "premultiplied",
                }});
            }}
            window.addEventListener("resize", resize);
            resize();

            let frameCount = 0;
            let lastTime = 0;

            function frame(timestamp) {{
                const time = Date.now() * 0.001;
                
                // FPS Counter
                if (timestamp - lastTime >= 1000) {{
                    console.log(`FPS: ${{frameCount}}`);
                    frameCount = 0;
                    lastTime = timestamp;
                }}
                frameCount++;

                // Update Uniforms
                uniformData[0] = mouseX;
                uniformData[1] = mouseY;
                uniformData[2] = canvas.width / (window.devicePixelRatio || 1);
                uniformData[3] = canvas.height / (window.devicePixelRatio || 1);
                uniformData[4] = time;
                device.queue.writeBuffer(uniformBuffer, 0, uniformData);

                const commandEncoder = device.createCommandEncoder();

                // Compute Pass
                const passEncoder = commandEncoder.beginComputePass();
                passEncoder.setPipeline(computePipeline);
                passEncoder.setBindGroup(0, bindGroup);
                const workgroupCount = Math.ceil(NUM_PARTICLES / 64);
                passEncoder.dispatchWorkgroups(workgroupCount);
                passEncoder.end();

                // Render Pass
                const textureView = context.getCurrentTexture().createView();
                const renderPass = commandEncoder.beginRenderPass({{
                    colorAttachments: [{{
                        view: textureView,
                        clearValue: {{ r: 0.05, g: 0.05, b: 0.1, a: 1.0 }},
                        loadOp: "clear",
                        storeOp: "store",
                    }}]
                }});
                renderPass.setPipeline(renderPipeline);
                renderPass.setBindGroup(0, renderBindGroup);
                renderPass.draw(6, NUM_PARTICLES, 0, 0);
                renderPass.end();

                device.queue.submit([commandEncoder.finish()]);
                requestAnimationFrame(frame);
            }}
            requestAnimationFrame(frame);
        }}
        initWebGPU();
    </script>
</body>
</html>
    """
    
    with open(os.path.join(OUTPUT_DIR, "index.html"), "w") as f:
        f.write(html_content)
    print("Created index.html")

if __name__ == "__main__":
    main()
