#version 410 core

layout (location = 0) in vec3 vertex;           // Vertex position
layout (location = 1) in vec3 normal;           // Vertex normal
layout (location = 2) in vec2 texCoord;         // Texture coordinates (changed to vec2)

out vec3 Normal;       // Pass normal vector to fragment shader
out vec3 viewPos;      // Pass view vector to fragment shader
out vec3 light;        // Pass light vector to fragment shader
out vec2 TexCoord;     // Pass texture coordinates to fragment shader

uniform mat4 vertex_model_to_world;    // Model matrix
uniform mat4 normal_model_to_world;    // Normal matrix (inverse transpose of model matrix)
uniform mat4 vertex_world_to_clip;     // Projection * View matrix
uniform vec3 light_position;           // Position of the light source
uniform vec3 camera_position;          // Position of the camera/viewer

void main()
{
    // Calculate world-space position of the vertex
    vec3 worldPos = (vertex_model_to_world * vec4(vertex, 1.0)).xyz;

    // Transform normal to world space
    Normal = (normal_model_to_world * vec4(normal, 1.0)).xyz;

    // Calculate the view vector (from the fragment to the camera)
    viewPos = camera_position - worldPos;

    // Calculate the light vector (from the fragment to the light source)
    light = light_position - worldPos;

    // Pass the texture coordinates to the fragment shader
    TexCoord = texCoord;

    // Calculate the final position of the vertex in clip space
    gl_Position = vertex_world_to_clip * vec4(worldPos, 1.0);
}
