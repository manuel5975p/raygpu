#version 460

// Input vertex attributes
layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec2 vertexTexCoord;
layout(location=2) in vec2 vertexTexCoord2;
layout(location=3) in vec4 vertexColor;

// Input uniform values
layout(binding = 0) uniform PerspectiveViewBlock {
    mat4 Perspective_View;
};
layout(binding = 1) uniform matModelBlock {
    mat4 matModel;
};

// Output vertex attributes (to fragment shader)
layout(location=0) out vec3 fragPosition;
layout(location=1) out vec2 fragTexCoord;
layout(location=2) out vec2 fragTexCoord2;
layout(location=3) out vec4 fragColor;

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragTexCoord2 = vertexTexCoord2;
    fragColor = vertexColor;

    // Calculate final vertex position
    gl_Position = Perspective_View * vec4(vertexPosition, 1.0);
}
