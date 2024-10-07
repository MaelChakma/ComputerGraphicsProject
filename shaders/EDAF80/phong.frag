#version 410

out vec4 FragColor;  // Final color of the fragment

in vec3 Normal;     // fN
in vec3 light;   // fL
in vec3 viewPos;    // fV

uniform vec3 diffuse; //kd
uniform vec3 ambient;  //ka
uniform vec3 specular;  //ks
uniform float shininess;            

uniform sampler2D diffuse_texture;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light);
    vec3 V = normalize(viewPos);
    vec3 R = normalize(reflect(-lightDir, norm));

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse_ = diffuse * diff;
    float spec = pow(max(dot(R, V), 0.0), shininess);
    vec3 specular_ = specular * spec;

    // Combine all components
    vec3 result = vec3(ambient + diffuse_ + specular_);
    FragColor.xyz = result;
    FragColor.w = 1.0;

    //FragColor = texture(diffuse_texture, fs_in.vertex)
}
