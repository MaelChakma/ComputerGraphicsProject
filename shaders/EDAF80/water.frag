#version 410 core

in vec2 TexCoord;  // Texture coordinates from vertex shader
in vec3 FragPos;   // Fragment position (world-space)

out vec4 FragColor;

uniform sampler2D water_texture;   // Water texture
uniform vec3 light_position;       // Light position for lighting
uniform vec3 camera_position;      // Camera position for specular highlights

void main()
{
    // Sample the water texture
    vec3 waterColor = texture(water_texture, TexCoord).rgb;

    // Simple diffuse lighting (Lambertian)
    vec3 lightDir = normalize(light_position - FragPos);
    vec3 normal = vec3(0.0, 1.0, 0.0);  // Default normal for a flat surface
    float diff = max(dot(lightDir, normal), 0.0);

    // Calculate view direction
    vec3 viewDir = normalize(camera_position - FragPos);

    // Combine water color with diffuse lighting
    vec3 result = waterColor * diff;

    FragColor = vec4(result, 1.0);
}
