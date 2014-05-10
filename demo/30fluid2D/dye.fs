#version 140

in vec2 v_texel;

uniform sampler2D tex_dye;
uniform vec2 mouse_pos;

out vec4 result;

void main()
{
	vec2 dye = texture(tex_dye, v_texel).xy;
	float dx = v_texel.x - mouse_pos.x;
	float dy = v_texel.y - mouse_pos.y;
	vec2 add = vec2(1.0, 0.2) * exp(-1000.0 * (dx * dx + dy * dy));
	result.xy = dye + add;
	
	result.zw = vec2(0.0);
}