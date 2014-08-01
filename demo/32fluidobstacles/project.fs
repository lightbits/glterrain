#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_pressure;
uniform sampler2D tex_obstacles;
uniform float dx;

out vec4 result;

void main()
{
	// Because this is the last stage of the velocity calculation
	// we also apply boundary conditions here
	float solid = texture(tex_obstacles, v_texel).r;
	if (solid < 1)
	{
		// Stuff can't move inside the obstacles
		result = vec4(0.0);
		return;
	}

	// Neighbor pressure values
	float pr = texture(tex_pressure, v_texel + vec2(dx, 0.0)).r;
	float pl = texture(tex_pressure, v_texel - vec2(dx, 0.0)).r;
	float pt = texture(tex_pressure, v_texel + vec2(0.0, dx)).r;
	float pb = texture(tex_pressure, v_texel - vec2(0.0, dx)).r;
	float pc = texture(tex_pressure, v_texel).r;

	// Neighbor obstacle values
	float or = texture(tex_obstacles, v_texel + vec2(dx, 0.0)).r;
	float ol = texture(tex_obstacles, v_texel - vec2(dx, 0.0)).r;
	float ot = texture(tex_obstacles, v_texel + vec2(0.0, dx)).r;
	float ob = texture(tex_obstacles, v_texel - vec2(0.0, dx)).r;

	// Use center pressure at obstacle
	if (or < 1) { pr = pc; }
	if (ol < 1) { pl = pc; }
	if (ot < 1) { pt = pc; }
	if (ob < 1) { pb = pc; }

	vec2 grad = vec2(pr - pl, pt - pb) * (1.0 / (2.0 * dx));
	// vec2 grad = vec2(pr - pl, pt - pb) * (1.125 / dx);
	vec2 u = texture(tex_velocity, v_texel).xy;
	result.xy = u - grad;
	result.zw = vec2(0.0);

	if (or < 1) { result.x = 0.0;}
	if (ol < 1) { result.x = 0.0;}
	if (ot < 1) { result.y = 0.0;}
	if (ob < 1) { result.y = 0.0;}
}