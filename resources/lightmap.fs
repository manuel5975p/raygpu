#version 460

// Input vertex attributes (from vertex shader)
layout(location=0) in vec2 fragTexCoord;
layout(location=1) in vec2 fragTexCoord2;
layout(location=2) in vec3 fragPosition;
layout(location=3) in vec4 fragColor;

// Input uniform values
layout(binding=2) uniform sampler2D texture0;
layout(binding=3) uniform sampler2D texture1;

// Output fragment color
layout(location=0) out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 texelColor2 = texture(texture1, fragTexCoord2);

    finalColor = texelColor * texelColor2;
}
