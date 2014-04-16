#version 140

uniform float light_r; // Light radius
uniform vec3 light_p; // Light position in world
uniform vec3 light_d; // Diffuse color
uniform vec3 light_s; // Specular color

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D tex_p; // Position texture
uniform sampler2D tex_n; // Normal texture
uniform sampler2D tex_d; // Diffuse texture

out vec4 out_color;

const vec3 ks = vec3(1.0, 1.0, 1.0); // Specular reflection
const vec3 kd = vec3(1.0, 1.0, 1.0); // Diffuse reflection
const float se = 100.0f; // Specular exponent

vec3 phong(in vec3 P, in vec3 N)
{
	vec3 D = (view * vec4(light_p, 1.0)).xyz - P;
	float dist = length(D);
	vec3 L = D / dist;							// Direction to light

	// Diffuse
	float NdotL = max(dot(N, L), 0.0);
	vec3 Id = NdotL * kd * light_d;				// Diffuse intensity

	// Specular
	vec3 R = reflect(-L, N);					// Reflected light direction
	vec3 V = normalize(-P);						// Direction to viewer
	float RdotV = max(dot(R, V), 0.0);
	float specular = pow(RdotV, se);
	vec3 Is = specular * ks * light_s;			// Specular intensity

	// Attenuation
	// float attenuation = 1.0 - (dist / light_r);
	float attenuation = -log(min(1.0, dist / light_r));

	return (Is + Id) * attenuation;
}

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / 720.0;
	uv.y = gl_FragCoord.y / 480.0;
	
	vec3 P = (texture(tex_p, uv)).xyz; // Vertex position in view-space
	if (P.z > -0.001) // Drop background
		discard;

	vec3 N = (texture(tex_n, uv)).xyz; // Vertex normal in view-space
	N = normalize(N);

	vec3 C = (texture(tex_d, uv)).xyz; // Diffuse color

	out_color.rgb = phong(P, N) * C;
	out_color.a = 1.0;

	// out_color *= 0.001;
	// out_color += vec4(1.0);
}