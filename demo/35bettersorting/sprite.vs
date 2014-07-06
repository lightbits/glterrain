#version 140

in vec4 position;

out vec4 vColor;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vec4 viewPos = view * model * vec4(position.xyz, 1.0);

	if (position.z < 0.0)
		vColor = vec4(1.0, 0.4, 0.4, 0.1);
	else
		vColor = vec4(0.4, 1.0, 0.4, 0.1);
	vColor.rgb *= exp(-length(viewPos) * 0.4 + 1.0);
	vColor.a *= exp(-length(viewPos) * 0.1 + 1.0);
	gl_PointSize = 1.0 - length(viewPos) / 6.0;
	gl_PointSize *= 100.0;

	gl_Position = projection * viewPos;
}