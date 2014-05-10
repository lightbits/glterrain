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

	// float dx = v_texel.x - 0.5;
	// float dy = v_texel.y - 0.5;
	// 
	// vec2 f = vec2(cos(time), sin(time)) * exp(-5.0 * (dx * dx + dy * dy));

	float len = length(v_texel - mouse_pos);
	vec2 f = mouse_vel * 100.0 * exp(-10.0 * len);

	result.xy = u + f * dt;
	result.zw = vec2(0.0);
}