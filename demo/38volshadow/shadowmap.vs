#version 430

in vec4 position;

out vec4 mask0;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	vec4 viewPos = view * position;
	gl_Position = projection * viewPos;
	gl_PointSize = 20.0;

	// float z = (viewPos.z + 1.0) / (-3.0 + 1.0);
	float z = 0.5 + 0.5 * gl_Position.z / gl_Position.w;
	mask0 = vec4(0.0);

	/*
	if (z >= 0.75) mask0.w = 1.0;
	else if (z >= 0.50) mask0.z = 1.0;
	else if (z >= 0.25) mask0.y = 1.0;
	else if (z >= 0.00) mask0.x = 1.0;
	*/	
	// GPUs don't like branches!
	mask0 = clamp(floor( mod(vec4(z) + vec4(1.00, 0.75, 0.50, 0.25), vec4(1.25)) ), vec4(0.0), vec4(1.0));

	/*
	mask0.x = clamp(floor(mod(z + 1.00, 1.25)), 0.0, 1.0);
	mask0.y = clamp(floor(mod(z + 0.75, 1.25)), 0.0, 1.0);
	mask0.z = clamp(floor(mod(z + 0.50, 1.25)), 0.0, 1.0);
	mask0.w = clamp(floor(mod(z + 0.25, 1.25)), 0.0, 1.0);
	*/
}
