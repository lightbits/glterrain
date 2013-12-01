#version 140

in vec2 vertPosition;
in vec2 vertTexel;

uniform vec3 light0Position;
uniform vec3 light0Color;
uniform vec3 ambient;
uniform float time;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform mat4 view;
out vec4 outColor;

void main()
{
	// Fetch normal from bumpmap texture
	float sampleFreq = 0.3;
	float damping = vertPosition.y;
	vec4 normal = texture(tex1, vertTexel * sampleFreq + time * 0.01);
	// vec4 normal = texture(tex1, vertTexel * sampleFreq * damping + time * 0.01) * 2.0 - vec4(1.0);
	normal.w = 0.0;

	// Transform normal to view space
	// normal = normalize(view * vec4(0.0, 1.0, 0.0, 0.0));

	vec2 texel = vertTexel;
	texel += normal.xy * 0.04;
	texel += vec2(sin(time * 3.0 + vertPosition.y * 5.0), 0.0) * 0.012 * pow(damping, 0.3);
	vec4 c1 = texture(tex0, texel + vec2(0.0, +1.0 / 480.0));
	vec4 c2 = texture(tex0, texel + vec2(0.0, -1.0 / 480.0));
	vec4 c3 = texture(tex0, texel + vec2(+1.0 / 640.0, 0.0));
	vec4 c4 = texture(tex0, texel + vec2(-1.0 / 640.0, 0.0));
	outColor = (c1 + c2 + c3 + c4) / 4.0;
	outColor *= 0.9;

	// outColor = vec4(normal.xyz * 0.5 + 0.5, 1.0);
	
	// vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.0));
	// float intensity = max(0, dot(lightDirection, normal.xyz));
	// outColor *= vec4(intensity * light0Color + (1.0 - intensity) * ambient, 1.0);
}