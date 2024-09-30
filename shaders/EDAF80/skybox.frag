#version 410

uniform samplerCube cubemap;

in VS_OUT {
	vec3 coordinates;
} fs_in;

out vec4 frag_color;

void main()
{
	//vec3 L = normalize(light_position - fs_in.vertex);
	frag_color = texture(cubemap, fs_in.coordinates);
}
