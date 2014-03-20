#version 140

in vec2 vertTexel;

out vec4 outColor;

// uniform float time;
uniform sampler2D tex;

void main()
{
	vec2 screenXY = vertTexel * 2.0 - 1.0;
	float r = length(screenXY);
	float vignette = exp(-pow(r, 5) * 0.2);
	// vec4 color = texture(tex, vertTexel);
	vec4 color = (
			   0.100 * texture(tex, vertTexel + vec2(-5 / 640.0, -5 / 480.0)) + 
			   0.175 * texture(tex, vertTexel + vec2(-0 / 640.0, -5 / 480.0)) + 
			   0.100 * texture(tex, vertTexel + vec2(+5 / 640.0, -5 / 480.0)) + 
			   0.175 * texture(tex, vertTexel + vec2(-5 / 640.0, -0 / 480.0)) + 
			   0.800 * texture(tex, vertTexel + vec2(-0 / 640.0, -0 / 480.0)) + 
			   0.175 * texture(tex, vertTexel + vec2(+5 / 640.0, -0 / 480.0)) + 
			   0.100 * texture(tex, vertTexel + vec2(-5 / 640.0, +5 / 480.0)) + 
			   0.175 * texture(tex, vertTexel + vec2(-0 / 640.0, +5 / 480.0)) + 
			   0.100 * texture(tex, vertTexel + vec2(+5 / 640.0, +5 / 480.0))
			   ) / 2.0;
	color *= vignette;
	outColor = color;
}