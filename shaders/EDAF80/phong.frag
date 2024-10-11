#version 410 

out vec4 FragColor;  // Final color of the fragment

in vec3 Normal;       // fN (fragment normal)
in vec3 light;        // fL (light direction)
in vec3 viewPos;      // fV (view direction)
in vec2 TexCoord;     // Texture coordinates passed from the vertex shader
in vec3 tangents;
in vec3 binormals;

uniform vec3 diffuse;   // kd
uniform vec3 ambient;   // ka
uniform sampler2D specular;  // ks
uniform float shininess;            
uniform sampler2D specular_map;
uniform sampler2D normal_map;
uniform sampler2D diffuse_texture;  // Texture sampler
uniform int has_diffuse_texture;    // Flag to check if there's a texture
uniform int use_normal_mapping;

void main()
{

    mat3 TBN = mat3(normalize(tangents), normalize(binormals), normalize(Normal));
    vec3 specular_ = texture(specular_map,TexCoord).xyz;
    // Normalize vectors
    vec3 norm = normalize(Normal);

    if (use_normal_mapping ==1 ){

        vec3 textureColor = texture(normal_map,TexCoord).xyz;
        norm = normalize(TBN * (textureColor * 2 - 1));

    }

    vec3 lightDir = normalize(light);
    vec3 viewDir = normalize(viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);


    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specularLight = specular_ * spec;

    float  diff = max(dot(norm, lightDir), 0.0);
    vec3  diffuseLight = diffuse * diff; 

    if (has_diffuse_texture == 1)
    {
        vec3 textureColor = texture(diffuse_texture, TexCoord).xyz;  
        diff = max(dot(norm, lightDir), 0.0);
        diffuseLight =diffuse *textureColor * diff;

    }

    vec3 finalColor = ambient + diffuseLight + specularLight;
    FragColor = vec4(finalColor, 1.0); 
}