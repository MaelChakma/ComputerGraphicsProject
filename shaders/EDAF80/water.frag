#version 410 core

in VS_OUT {
    vec3 vertex;   // World-space position from vertex shader
    vec3 normal;   // World-space normal from vertex shader
} fs_in;

out vec4 FragColor;

uniform sampler2D normal_map;   // Normal map for the water surface
uniform sampler2D water_texture;  // Water texture (optional)
uniform vec3 light_position;    // Light position in world space
uniform vec3 camera_position;   // Camera position in world space

void main()
{
    // Sample the normal map (optional, assuming 2D texture coordinates are available)
    vec3 normal_map_sample = texture(normal_map, vec2(fs_in.vertex.x, fs_in.vertex.z)).rgb;
    vec3 normal_tangent = normalize(normal_map_sample * 2.0 - 1.0);  // Convert to [-1, 1] range

    // Since the normal from the vertex shader is in world space, we use it directly
    vec3 normal_world = normalize(fs_in.normal);

    // Lighting calculations
    vec3 light_dir = normalize(light_position - fs_in.vertex);
    vec3 view_dir = normalize(camera_position - fs_in.vertex);

    // Simple Lambertian diffuse lighting
    float diff = max(dot(normal_world, light_dir), 0.0);

    // Combine the water texture (optional) with the lighting
    vec3 water_color = texture(water_texture, vec2(fs_in.vertex.x, fs_in.vertex.z)).rgb;
    vec3 final_color = water_color * diff;

    FragColor = vec4(final_color, 1.0);
}
