#version 140

in vec2 position; // Fullscreen quad coordinates [-1, 1] on both axes

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec2 scale; // Scale of quad in world-space

void main()
{
	// Depth of quad in view-space
	float z_view = (view * model * vec4(0, 0, 0, 1)).z;

	// Range of quad in clip-space
	vec4 r_clip = projection * vec4(scale.x / 2.0, scale.y / 2.0, 0.0, 1.0);

	/*
	          r_clip.x
	          +------+
              .
		+------------+....|
		|        __/ |    | r_clip.y
		|    ___/    |....|
		| __/        |
		|/           |
		+------------+

	I project the range of the quad into clip-coordinates, and use the result
	to scale the fullscreen quad coordinates into a viewer-oriented quad, with
	correct scale.

	x0 = center.x + (-1) * r_clip.x
	y0 = center.y + (+1) * r_clip.y

	x1 = center.x + (+1) * r_clip.x
	... and so on,
	where center is the center of the quad in clip-coordinates.

	The coefficient in front of r_clip is stored in the position attribute.

	Because we output clip-coordinates directly, we need to store the z-coordinate
	in view-space (NDC coordinates are formed by dividing clip-coords by the w component).
	*/

	vec4 c_clip = projection * view * model * vec4(0.0, 0.0, 0.0, 1.0);
	gl_Position = vec4(
	    c_clip.x + position.x * r_clip.x, 
	    c_clip.y + position.y * r_clip.y, 
		c_clip.z, 
		-z_view);
}