#version 410 core

out vec4 FragColor;  // Final color of the fragment

in vec3 Normal;       // Fragment normal (passed from vertex shader)
in vec3 light;        // Light direction
in vec3 viewPos;      // View direction
in vec2 TexCoord;     // Texture coordinates passed from vertex shader

uniform vec3 diffuse;   // Diffuse color (kd)
uniform vec3 ambient;   // Ambient color (ka)
uniform sampler2D specular;  // Specular map (ks)
uniform float shininess;     // Shininess factor

uniform sampler2D diffuse_texture;  // Diffuse texture sampler
uniform sampler2D normal_map;       // Normal map for normal mapping
uniform int has_diffuse_texture;    // Flag to check if there's a texture
uniform int has_normal_map;         // Flag to check if there's a normal map

void main()
{
    // Sample specular map
    vec3 specular_ = texture(specular, TexCoord).xyz;
    vec3 norm = normalize(Normal);

    if (has_normal_map == 1)
    {
        vec3 normal_map_sample = texture(normal_map, TexCoord).xyz * 2.0 - 1.0;
        norm = normalize(normal_map_sample);
    }

    // Normalize light and view directions
    vec3 lightDir = normalize(light);
    vec3 viewDir = normalize(viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Diffuse component
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffuse * diff;
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specularLight = specular_ * spec;

    vec3 finalColor = ambient + diffuseLight + specularLight;

    if (has_diffuse_texture == 1)
    {
        vec3 textureColor = texture(diffuse_texture, TexCoord).xyz;
        finalColor *= textureColor;  
    }

    // Output final color
    FragColor = vec4(finalColor, 1.0);
}
