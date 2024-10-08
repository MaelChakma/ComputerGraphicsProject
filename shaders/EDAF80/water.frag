#version 410

uniform vec3 light_position;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{

	vec4 colordeep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 colorshallow = vec4(0.0, 0.5, 0.5, 1.0);
	//vec4 colorwater = mix(colordeep, colorshallow, facing);

	vec3 L = normalize(light_position - fs_in.vertex);
	frag_color = vec4(1.0) * clamp(dot(normalize(fs_in.normal), L), 0.0, 1.0);
}
