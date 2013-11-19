#version 140

in vec2 vertUV;

out vec4 outColor;

uniform sampler2D heightmap;
uniform float time;

const vec3 eye = vec3(0.0, 0.4, -0.5);
const vec3 sky = vec3(0.6, 0.65, 0.89);
const vec3 forward = vec3(0.0, 0.0, 1.0);
const vec3 right = vec3(1.0, 0.0, 0.0);
const vec3 up = vec3(0.0, 1.0, 0.0);
const float focalLength = 1.3;
const float aspectRatio = 640.0 / 480.0;

const float mapNear = 0.0;
const float mapFar = 1.0;
const float mapLeft = -1.0;
const float mapRight = 1.0;
const float mapHeight = 1.0;

float getHeight(vec2 xy)
{
	return texture(heightmap, xy);
}

vec3 raymarch(vec3 ro, vec3 rd)
{
	float t = 0.0f;
	float dt = 0.016f;
	for(int i = 0; i < 128; ++i)
	{
		vec3 p = ro + rd * t;
		t += dt;
		if(p.x < mapLeft || p.x > mapRight || p.z < mapNear || p.z > mapFar)
			continue;

		vec2 texUV = vec2((p.x - mapLeft) / (mapRight - mapLeft), (p.z - mapNear) / (mapFar - mapNear));
		float height = texture(heightmap, texUV).r * mapHeight;
		if(p.y <= height)
		{
			return vec3(1.0 - p.z);
		}
	}

	return sky;
}

void main()
{
	vec3 ro = eye;
	vec3 rd = normalize(forward * focalLength + right * vertUV.x * aspectRatio + up * vertUV.y);
	// vec3 color = raymarch(ro, rd);
	// outColor = vec4(color, 1.0);

	float u = vertUV.x * 0.5 + 0.5;
	float v = vertUV.y * 0.5 + 0.5;
	int x = int(cos(u * 5.0) * 30.0);
	int y = int(tan(v * 5.0) * cos(u) * 30.0);
	int ms = x ^ y;
	int n = int(mod(time * 10, 50.0));
	if(ms < n)
	{
		outColor = vec4(0.2588235, 0.227451, 0.1960784, 1.0);
		if(mod(x, 4) < 2)
			outColor = vec4(0.2588235, 0.227451, 0.1960784, 1.0);
		if(mod(y, 4) < 2)
			outColor = vec4(0.6039216, 0.4078431, 0.5647059, 1.0);
	}
	else
	{
		outColor = vec4(0.8, 0.8, 0.8, 1.0);
	}
	// outColor = texture(heightmap, uv * 0.5 + vec2(0.5));
	// outColor = vec4(uv.x * 0.5 + 0.5, uv.y * 0.5 + 0.5, 0.0, 1.0);
}