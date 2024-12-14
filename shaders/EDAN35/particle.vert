#version 410 core

layout (location = 0) in vec3 vertexPos;  // Vertex position
layout (location = 1) in vec2 texCoords;


// Output to fragment shader
out VS_OUT {
    vec2 texCoords;
    vec4 particleColor;
} vs_out;


uniform mat4 projection;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    float scale = 10.0f;
    vs_out.texCoords = texCoords;
    vs_out.particleColor = color;
    gl_Position = projection * vec4((vertexPos.xyz * scale) + offset, 1.0);
}
