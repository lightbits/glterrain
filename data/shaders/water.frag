#version 140

in vec2 vertPosition;
in vec2 vertTexel;

uniform vec3 light0Position;
uniform vec3 light0Color;
uniform vec3 ambient;
uniform float time;

uniform sampler2D tex;

out vec4 outColor;

void main()
{
	vec2 texelOffset = 0.003 * vec2(sin(time * 5.0 + vertPosition.y * 80.0));
	outColor = texture(tex, vertTexel + texelOffset) * vec4(0.8, 0.8, 0.8, 0.9);
	// outColor = texture(tex, vertTexel + 0.003 * vec2(sin(time * 30.0 + vertPosition.y * 100.0) + 0.2 * cos(vertPosition.y * 50.0 + time), cos(time)));
	
	// vec3 lightDirection = normalize(vec3(0.0, 0.1, 0.0) - worldPos.xyz);
	// float intensity = max(0, dot(lightDirection, vec3(0.0, 1.0, 0.0)));
	// outColor = vec4(intensity * light0Color + (1.0 - intensity) * ambient, 1.0);
	// outColor *= vertColor;

  	// // Fog
	// const float LOG2 = 1.442695;
	// const float density = 0.05125;
	// vec4 fogColor = vec4(0.77, 0.68, 0.68, 1.0);
	// float z = distToCamera;
	// float fogFactor = 1.0 - clamp(exp2(-density * density * z * z * LOG2), 0.0, 1.0);
	// outColor = mix(outColor, fogColor, fogFactor);
}