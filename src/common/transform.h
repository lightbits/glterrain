#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <common/matrix.h>
#include <common/vec.h>

namespace transform
{

mat4 rotateX(float rad);
mat4 rotateY(float rad);
mat4 rotateZ(float rad);
mat4 translate(float x, float y, float z);
mat4 translate(const vec3 &v);
mat4 scale(float x, float y, float z);
mat4 scale(float s);
mat4 perspective(float fov_y, float aspect_ratio, float z_near, float z_far);

}

#endif