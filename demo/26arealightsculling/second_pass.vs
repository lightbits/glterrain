#version 140

in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

mat4 scale(float x, float y, float z)
{
	mat4 m = mat4(1.0);
	m[0][0] = x;
	m[1][1] = y;
	m[2][2] = z;
	return m;
}

mat4 translateY(float y)
{
	mat4 m = mat4(1.0);
	m[3].y = y;
	return m;
}

void main()
{
	gl_Position = projection * view * model * translateY(-4.0) * scale(4.0, 8.0, 8.0) * vec4(position, 1.0);
}