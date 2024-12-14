#version 450 core

// Quad geometry (shared by all particles)
layout(location = 0) in vec3 quadPosition;

// Per-instance attributes
layout(location = 1) in vec3 positionIn;
layout(location = 2) in vec3 velocityIn;
layout(location = 3) in float lifetimeIn;
layout(location = 4) in vec4 colorIn;

// Uniform for time step
uniform float deltaTime;

out vec3 positionOut;
out vec3 velocityOut;
out float lifetimeOut;
out vec4 colorOut; // Passed to the fragment shader

void main() {
    // Calculate particle position based on velocity and delta time
    vec3 updatedPosition = positionIn + 0.1f * deltaTime;

    // Final position of the vertex for this instance
    gl_Position = vec4(quadPosition + updatedPosition, 1.0);

    // Pass color or other properties to the fragment shader
    colorOut = vec4(0.0, 0.0, max(lifetimeIn,0.0), 1.0 );
    //colorOut = vec4(1.0, 1.0, 1.0, 1.0);

	positionOut = updatedPosition;
	velocityOut = velocityIn;
	lifetimeOut = lifetimeIn-1.0f;
}
