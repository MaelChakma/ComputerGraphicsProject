#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal; 

out vec3 FragPos;   // Position of the fragment
out vec3 Normal;    // Normal vector of the fragment

uniform mat4 vertex_model_to_world;     // vertex_model_to_world matrix
uniform mat4 normal_model_to_world;       // View matrix
uniform mat4  vertex_world_to_clip; // Projection matrix

void main()
{
    // Calculate the fragment position in world space
    FragPos = vec3(vertex_model_to_world* vec4(vertex, 1.0));
    
    // Pass the normal vector transformed by the normal matrix (inverse transpose of the vertex_model_to_world matrix)
    Normal = mat3(transpose(inverse(vertex_model_to_world))) * normal;  
    
    // Calculate the final position of the vertex (in clip space)
    gl_Position =  vertex_world_to_clip * normal_model_to_world * vec4(vertex, 1.0);
}
