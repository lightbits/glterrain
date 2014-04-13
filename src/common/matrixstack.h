#ifndef SLGL_COMMON_MATRIX_STACK_H
#define SLGL_COMMON_MATRIX_STACK_H
#include <stack>
#include <common/matrix.h>
#include <common/quaternion.h>
#include <common/vec.h>
#include <common/transform.h>

/*
A matrix stack allows you to hierarchically perform matrix transformations, accumulating
transformations as we go along.
The stack is initialized with the identity matrix, and the top matrix is
the current transformation matrix.

The current matrix can be modified through various functions, such as rotate* and
translate(x, y, z). The result is a new current matrix, formed by right-multiplying
the current matrix with the transformation matrix.

Push will save this matrix on the stack, and leave the current matrix on the top.
Pop will restore the earlier matrix, by setting the current matrix to be whatever
is on the top, and removing top from the stack.

The effect of this is that you can save a matrix, modify the current matrix, and restore
the old matrix. For example, you may have a hierarchy of transformations such as an initial
translation and rotation
	
	stack.translate(1, 2, 1);
	stack.rotateY(45);

	all further transformations are affected by the above

		stack.push(); - save the current "parent" transformation
		stack.rotateZ(30);
		v0 = stack.top() * v0;
		stack.pop();

		stack.push(); - save the current "parent" transformation
		stack.translate(2, 2, 3);
		v1 = stack.top() * v1;
		stack.pop();

the effect of this is that v0 is first rotated around the z axis, and then
rotated around the y axis and finally translated.
v1 will first be translated to 2, 2, 3, and then rotated around the y axis
and finally translated (1, 1, 1) units relative to that position.

*/
class MatrixStack
{
public:
	// Initializes the stack with the identity matrix
	MatrixStack() : current(1.0f)
	{

	}

	// Applies a rotation transformation to the current matrix
	void rotateX(float rad)
	{
		float co = cosf(rad); float si = sinf(rad);
		mat4 m(1.0f);
		m[1][1] = co; m[1][2] = -si; m[2][1] = si; m[2][2] = co;
		current = current * m;
	}

	void rotateY(float rad)
	{
		float co = cosf(rad); float si = sinf(rad);
		mat4 m(1.0f);
		m[0][0] = co; m[0][2] = si; m[2][0] = -si; m[2][2] = co;
		current = current * m;
	}

	void rotateZ(float rad)
	{
		float co = cosf(rad); float si = sinf(rad);
		mat4 m(1.0f);
		m[0][0] = co; m[1][0] = -si; m[0][1] = si; m[1][1] = co;		
		current = current * m;
	}

	void translate(float x, float y, float z)
	{
		mat4 m(1.0f);
		m[3][0] = x; m[3][1] = y; m[3][2] = z; m[3][3] = 1.0f;
		current = current * m;
	}

	void translate(const vec3 &v)
	{
		mat4 mat(1.0f);
		mat[3] = vec4(v, 1.0f); // the fourth column
		current = current * mat;
	}

	void scale(float x, float y, float z)
	{
		mat4 mat(1.0f);
		mat[0].x = x;
		mat[1].y = y;
		mat[2].z = z;
		current = current * mat;
	}

	void scale(float s)
	{
		scale(s, s, s);
	}

	// Takes the current transformation matrix and left multiplies it with the argument
	void multiply(const mat4 &mat)
	{
		current = current * mat;
	}

	void multiply(const quat &q)
	{
		current = current * glm::mat4_cast(q);
	}

	// Takes the current matrix and pushes it onto the stack
	void push()
	{
		matrices.push(current);
	}

	// Sets the current matrix to be whatever the top of the stack is, and removes top from the stack
	void pop()
	{
		current = matrices.top();
		matrices.pop();
	}

	// Returns the current matrix
	const mat4 &top() const
	{
		return current;
	}
private:
	std::stack<mat4> matrices;
	mat4 current;
};

#endif