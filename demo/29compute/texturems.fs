#version 430
layout (binding = 0, rgba16f) uniform readonly image2D tex;
out vec4 outColor;

void main()
{
	vec2 uv = gl_FragCoord.xy;
	uv.x /= 500.0;
	uv.y /= 500.0;

	// Nearest sampling
	// ivec2 texel = ivec2(uv * imageSize(tex));
	// outColor = imageLoad(tex, texel);

	// Bilinear sampling
	vec2 texelF = uv * imageSize(tex);
	ivec2 texelI = ivec2(texelF);
	vec2 alpha = texelF - floor(texelF);
	vec4 a0 = imageLoad(tex, texelI);
	vec4 a1 = imageLoad(tex, texelI + ivec2(1, 0));
	vec4 b0 = imageLoad(tex, texelI + ivec2(0, 1));
	vec4 b1 = imageLoad(tex, texelI + ivec2(1, 1));
	vec4 c0 = mix(a0, a1, alpha.x);
	vec4 c1 = mix(b0, b1, alpha.x);
	outColor = mix(c0, c1, alpha.y);
}