#version 150

in vec3 position;
in vec4 color;
in vec2 texel;

out vec4 vertColor;
out vec2 vertTexCoord;

uniform mat4 projection;
uniform mat4 view;

void main()
{	
	vertColor = color;
	vertTexCoord = texel;
	gl_Position = projection * view * vec4(position.x, position.y, position.z, 1.0f);
}