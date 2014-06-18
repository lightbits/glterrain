#version 430

uniform sampler2D texFront;
uniform sampler2D texBack;
uniform sampler3D texVolume;

in vec2 vTexel;

out vec4 outColor;

const float stepSize = 0.01;

void main()
{
	vec3 front = texture(texFront, vTexel).xyz;
	vec3 back = texture(texBack, vTexel).xyz;

	if (front == back)
	{
		discard;
		return;
	}

	vec3 rayDistance = back - front;
	vec3 rayOrigin = front;
	float rayLength = length(rayDistance);
	vec3 step = stepSize * (rayDistance / rayLength);

	vec3 pos = front;
	vec4 dst = vec4(0.0);
	while (dst.a < 1.0 && rayLength > 0)
	{
		float density = texture(texVolume, pos).x;
		vec4 src = vec4(density);
		src.rgb *= src.a;
		dst = (1.0 - dst.a) * src + dst;
		pos += step;
		rayLength -= stepSize;
	}

	outColor = dst;
	outColor.g *= 0.94;
	outColor.b *= 0.89;
}