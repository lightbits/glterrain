#include <common/transform.h>

mat4 transform::rotateX(float rad)
{
	float co = cosf(rad); float si = sinf(rad);
	mat4 m(1.0f);
	m[1][1] = co; m[1][2] = -si; m[2][1] = si; m[2][2] = co;
	return m;
}

mat4 transform::rotateY(float rad)
{
	float co = cosf(rad); float si = sinf(rad);
	mat4 m(1.0f);
	m[0][0] = co; m[0][2] = si; m[2][0] = -si; m[2][2] = co;
	return m;
}

mat4 transform::rotateZ(float rad)
{
	float co = cosf(rad); float si = sinf(rad);
	mat4 m(1.0f);
	m[0][0] = co; m[1][0] = -si; m[0][1] = si; m[1][1] = co;
	return m;
}

mat4 transform::translate(float x, float y, float z)
{
	mat4 m(1.0f);
	m[3][0] = x; m[3][1] = y; m[3][2] = z; m[3][3] = 1.0f;
	return m;
}

mat4 transform::translate(const vec3 &v)
{
	mat4 m(1.0f);
	m[3] = vec4(v, 1.0f); // the fourth column
	return m;
}

mat4 transform::scale(float x, float y, float z)
{
	mat4 m(1.0f);
	m[0][0] = x; m[1][1] = y; m[2][2] = z;
	return m;
}

mat4 transform::scale(float s)
{
	return transform::scale(s, s, s);
}

mat4 transform::perspective(float fov_y, float aspect_ratio, float z_near, float z_far)
{
	return glm::perspective(fov_y, aspect_ratio, z_near, z_far);
}