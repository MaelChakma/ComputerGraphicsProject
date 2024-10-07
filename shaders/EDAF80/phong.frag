#version 410 core

out vec4 FragColor;  // Final color of the fragment

in vec3 Normal;       // fN (fragment normal)
in vec3 light;        // fL (light direction)
in vec3 viewPos;      // fV (view direction)
in vec2 TexCoord;     // Texture coordinates passed from the vertex shader

uniform vec3 diffuse;   // kd
uniform vec3 ambient;   // ka
uniform sampler2D specular;  // ks
uniform float shininess;            

uniform sampler2D diffuse_texture;  // Texture sampler
uniform int has_diffuse_texture;    // Flag to check if there's a texture

void main()
{
    vec3 specular_ = texture(specular,TexCoord).xyz;
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light);
    vec3 viewDir = normalize(viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);


    // Diffuse component 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffuse * diff;

    // Specular component
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specularLight = specular_ * spec;

    vec3 finalColor = ambient + diffuseLight + specularLight;

    if (has_diffuse_texture == 1)
    {
        vec3 textureColor = texture(diffuse_texture, TexCoord).xyz;
        finalColor *= textureColor;  
    }

    FragColor = vec4(finalColor, 1.0); 
}