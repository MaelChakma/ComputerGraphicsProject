#version 410 core

layout(location = 0) in vec3 inPosition;  // Vertex position
layout(location = 1) in vec2 inTexCoord;  // Texture coordinates

out vec2 TexCoord;                       // Texture coordinates to fragment shader
out vec3 FragPos;                        // Fragment position (world-space)

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection; // Projection matrix
uniform float elapsed_time_s;  // Time uniform for wave animation

void main()
{
    // Calculate wave displacement (e.g., using sine waves)
    float waveHeight = sin(inPosition.x * 0.1 + elapsed_time_s) * 0.2 + sin(inPosition.z * 0.1 + elapsed_time_s * 0.7) * 0.2;

    // Modify the y-position to simulate the wave height
    vec3 displacedPosition = vec3(inPosition.x, waveHeight, inPosition.z);

    // Compute the final vertex position (world -> view -> clip space)
    gl_Position = projection * view * model * vec4(displacedPosition, 1.0);

    // Pass texture coordinates and world position to the fragment shader
    TexCoord = inTexCoord;
    FragPos = displacedPosition;
}
