#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	glm::vec3 v = (1 - x) * p0 + x * p1;
	return v;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	glm::vec4 m1 = glm::vec4( 1,x,x * x,x * x * x );
	glm::mat4 m2 = glm::mat4({ 0,-t,2 * t,-t }, {1,0,t-3,2-t}, {0,t,3-2*t,t-2}, {0,0,-t,t});
	glm::mat4x3 m3 = glm::transpose(glm::mat4x3( p0,p1,p2,p3 ));
	glm::vec3 pos = m1 * m2 * m3;



	return pos;
}
