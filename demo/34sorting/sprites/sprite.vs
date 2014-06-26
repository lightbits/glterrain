#version 140

in vec3 position;
in vec4 spritePosition;

out vec4 vColor;
out float radius;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vec4 viewPos = view * model * vec4(spritePosition.xyz + position, 1.0);
	//vColor = vec4(1.0, 0.5, 0.5, 0.1);

	if (spritePosition.z < 0.0)
		vColor = vec4(1.0, 0.4, 0.4, 0.1);
	else
		vColor = vec4(0.4, 1.0, 0.4, 0.1);

	radius = length(position.xy);
	vColor.rgb *= exp(-length(viewPos) * 0.4 + 1.0);
	vColor.a *= exp(-length(viewPos) * 0.1 + 1.0);
	// vColor *= 0.03;
	gl_Position = projection * viewPos;
}