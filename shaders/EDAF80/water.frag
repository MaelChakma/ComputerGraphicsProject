#version 410 core

in VS_OUT {
    vec3 vertex;   // World-space position from vertex shader
    vec3 normal;   // World-space normal from vertex shader
} fs_in;

out vec4 FragColor;

uniform sampler2D normal_map;   // Normal map for the water surface
uniform sampler2D water_texture; 
uniform vec3 light_position;    // Light position in world space
uniform vec3 camera_position;   // Camera position in world space

void main()
{
    vec3 normal_map_sample = texture(normal_map, vec2(fs_in.vertex.x, fs_in.vertex.z)).rgb;
    vec3 normal_tangent = normalize(normal_map_sample * 2.0 - 1.0); 

    vec3 normal_world = normalize(fs_in.normal);

    vec3 light_dir = normalize(light_position - fs_in.vertex);
    //vec3 view_dir = normalize(camera_position - fs_in.vertex);

    float diff = max(dot(normal_world, light_dir), 0.0);
	vec4 colordeep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 colorshallow = vec4(0.0, 0.5, 0.5, 1.0);

	FragColor = vec4(1.0) * clamp(diff, 0.0, 1.0);
}
