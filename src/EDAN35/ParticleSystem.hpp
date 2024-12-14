#pragma once

#include <glm/glm.hpp>
	using namespace glm;

#include <imgui.h>

struct Particle {
	vec3 position, velocity;
	vec4 color;
	float lifetime;

	Particle()
		: position(0.0f), velocity(0.0f), color(1.0f), lifetime(0.0f) {
	}
};


class ParticleSystem {

public:
	ParticleSystem();

	~ParticleSystem();

	bool InitParticleSystem(const vec3& Pos);

	void Render(int deltaT, const mat4& VP, const vec3& CameraPos);

private:

	bool m_isFirst;
	unsigned int m_currVB;
	unsigned int m_currTFB;
	int m_time;
};
