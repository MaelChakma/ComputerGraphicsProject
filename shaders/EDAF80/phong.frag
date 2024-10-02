#version 410

out vec4 FragColor;  // Final color of the fragment

in vec3 FragPos;     // Fragment position from vertex shader
in vec3 Normal;      // Fragment normal vector from vertex shader

uniform vec3 lightPos;   // Position of the light source
uniform vec3 viewPos;    // Position of the camera/viewer
uniform vec3 lightColor; // Color/intensity of the light
uniform vec3 objectColor; // Color of the object

// Phong lighting parameters
uniform float ambientStrength = 0.1;   // Ambient lighting strength
uniform float specularStrength = 0.5;  // Specular lighting strength
uniform int shininess = 32;            // Shininess of the surface

void main()
{
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting (Lambert's cosine law)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine all components
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
