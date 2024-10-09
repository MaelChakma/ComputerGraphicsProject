#version 410 core

layout (location = 0) in vec3 vertex;  // Vertex position
layout (location = 1) in vec3 normal;  // Normal vector

uniform mat4 vertex_model_to_world;    // Model matrix 
uniform mat4 normal_model_to_world;    // Normal matrix 
uniform mat4 vertex_world_to_clip;     // Projection * View matrix 
uniform float elapsed_time;            // Time uniform for wave animation

// Output to fragment shader
out VS_OUT {
    vec3 vertex;   // Transformed vertex 
    vec3 normal;   // Transformed normal 
} vs_out;

// Function to generate a wave displacement
float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
{
    return amplitude * pow(sin((position.x * direction.x + position.y * direction.y) * frequency + phase * time) * 0.5 + 0.5, sharpness);
}

float derivative(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
{
    return 0.5* sharpness * frequency * amplitude * pow(sin((position.x * direction.x + position.y * direction.y) * frequency + phase * time) * 0.5 + 0.5, sharpness-1) * cos(position.x * direction.x + position.y * direction.y);
}



void main()
{
    vec3 displaced_vertex = vertex; 
    displaced_vertex.y += wave(vertex.xz, vec2(-1.0, 0.0), 0.2, 3.0, 1.0, 2.0, elapsed_time);
    displaced_vertex.y += wave(vertex.xz, vec2(1.0, 0.5), 0.15, 2.0, 1.5, 3.0, elapsed_time);


    vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));



    vs_out.normal = vec3(derivative(vertex.xz, vec2(-1.0, 0.0), 0.2, 3.0, 1.0, 2.0, elapsed_time),1,derivative(vertex.xz, vec2(-1.0, 0.0), 0.2, 3.0, 1.0, 2.0, elapsed_time)*0) ;  
    vs_out.normal += vec3(derivative(vertex.xz, vec2(1.0, 0.5), 0.15, 2.0, 1.5, 3.0, elapsed_time),1,derivative(vertex.xz, vec2(1.0, 0.5), 0.15, 2.0, 1.5, 3.0, elapsed_time)*0.5);  
    
    
    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vs_out.vertex, 1.0);
}
