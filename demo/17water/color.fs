#version 140

in vec4 world_pos;
in vec4 vert_color;

out vec4 out_color;

void main()
{
	if (world_pos.y > 0.1)
		discard;
	out_color = vert_color;
	out_color.w = world_pos.y;
}