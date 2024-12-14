#version 410 core

in VS_OUT {
    vec2 texCoords;
    vec4 particleColor; 
} fs_in;


out vec4 color;

uniform sampler2D sprite;

void main()
{
    //color = (texture(sprite, fs_in.texCoords) * fs_in.particleColor);
	color = fs_in.particleColor;
}  
