#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform float dt;
uniform vec2 mouse_pos;
uniform vec2 mouse_vel;

out vec4 result;

void main()
{
	vec2 u = texture(tex_velocity, v_texel).xy;

	float len = length(v_texel - mouse_pos);
	vec2 f = mouse_vel * 200.0 * exp(-14.0 * len);

	result.xy = u + f * dt;
	result.zw = vec2(0.0);
}