/*
Matrix math including vectors and 4x4 matrices.
*/

#ifndef COMMON_MATRIX_H
#define COMMON_MATRIX_H
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::ortho and glm::perspective
#include <glm/gtc/type_ptr.hpp>	// For value_ptr(matrix)
#include <glm/gtc/quaternion.hpp>

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::ivec4 vec4i; // Signed integer
typedef glm::ivec3 vec3i; // Signed integer
typedef glm::ivec2 vec2i; // Signed integer
typedef glm::uvec4 vec4u; // Unsigned integer
typedef glm::uvec3 vec3u; // Unsigned integer
typedef glm::uvec2 vec2u; // Unsigned integer

typedef glm::quat quat;

static quat quaternion(float angle, const vec3 &axis)
{
	return glm::angleAxis(angle, axis);
}

static quat slerp(const quat &r, const quat &q, float t)
{
	return glm::slerp(r, q, t);
}

//struct quaternion
//{
//	/* 
//	Creates a quaternion from an angle-axis.
//	angle: Rotation angle about axis in radians 
//	axis: Normalized rotation axis
//	*/
//	quaternion(float angle, const vec3 &axis)
//	{
//		float s = sin(0.5f * angle);
//		w = cos(0.5f * angle);
//		x = s * axis.x;
//		y = s * axis.y;
//		z = s * axis.z;
//	}
//
//	/*
//	Creates a quaternion with the given components
//	*/
//	quaternion(float X, float Y, float Z, float W) :
//		x(X), y(Y), z(Z), w(W) 
//	{ }
//
//	void normalize()
//	{
//		float invlen = 1.0f / sqrt(w * w + x * x + y * y + z * z);
//		w *= invlen;
//		x *= invlen;
//		y *= invlen;
//		z *= invlen;
//	}
//
//	mat4 getMatrix() const
//	{
//		mat4 m;
//		m[0][0] = 1 - 2 * y * y - 2 * z * z;
//		m[1][0] = 2 * x * y - 2 * w * z;
//		m[2][0] = 2 * x * z + 2 * w * y;
//		m[3][0] = 0;
//
//		m[0][1] = 2 * x * y + 2 * w * z;
//		m[1][1] = 1 - 2 * x * x - 2 * z * z;
//		m[2][1] = 2 * y * z - 2 * w * x;
//		m[3][1] = 0;
//
//		m[0][2] = 2 * x * z - 2 * w * y;
//		m[1][2] = 2 * y * z + 2 * w * x;
//		m[2][2] = 1 - 2 * x * x - 2 * y * y;
//		m[3][2] = 0;
//
//		m[0][3] = 0;
//		m[1][3] = 0;
//		m[2][3] = 0;
//		m[3][3] = 1;
//		return m;
//	}
//	
//	quaternion operator*(const quaternion &b)
//	{
//		return quaternion(
//			w * b.y + y * b.w + z * b.x - x * b.z,
//			w * b.z + z * b.w + x * b.y - y * b.x,
//			w * b.w - x * b.x - y * b.y - z * b.z,
//			w * b.x + x * b.w + y * b.z - z * b.y
//			);
//	}
//
//	float x, y, z, w;
//};
//
//static quaternion slerp(const quaternion &q, const quaternion &r, float t)
//{
//	float dp = q.w * r.w + q.x * r.x + q.y * r.y + q.z * r.z;
//	float om = acos(dp);
//	float a = sin((1 - t) * om) / sin(om);
//	float b = sin(t * om) / sin(om);
//	return quaternion(
//		a * q.x + b * r.x,
//		a * q.y + b * r.y,
//		a * q.z + b * r.z,
//		a * q.w + b * r.w
//		);
//}

#endif