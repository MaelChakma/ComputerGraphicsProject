#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec3 texture_coordinates;
layout(location = 3) in vec3 tangent; 
layout(location = 4) in vec3 binormal; 

uniform int has_diffuse_texture;


out vec3 Normal;   // fn normal vector
out vec3 viewPos;    // fV view vector
out vec3 light;         // FL light vector
out vec3 vertex;

uniform mat4 vertex_model_to_world;     // vertex_model_to_world matrix
uniform mat4 normal_model_to_world;       // tbn View matrix
uniform mat4  vertex_world_to_clip; // Projection matrix
uniform vec3 light_position;
uniform vec3 camera_position;

void main()
{
    vec3 worldPos = (vertex_model_to_world* vec4(vertex, 1.0)).xyz;

    Normal = (normal_model_to_world*vec4(normal,1.0)).xyz;  
    viewPos = camera_position-worldPos;
    light = light_position - worldPos;
    gl_Position =  vertex_world_to_clip * normal_model_to_world * vec4(vertex, 1.0);
}
