#include "app.h"
#include <common/noise.h>
using namespace transform;

VertexArray vao;
ShaderProgram shader_intervals, shader_raycast;
Model cube;
MeshBuffer cube_buffer, ssquad_buffer;
mat4 mat_projection, mat_view;
RenderTexture tex_front, tex_back;
GLuint tex_volume;

vec3 mod289(vec3 x) {
	return x - glm::floor(x * vec3(1.0 / 289.0)) * 289.0f;
}

vec4 mod289(vec4 x) {
	return x - glm::floor(x * vec4(1.0f / 289.0f)) * 289.0f;
}

vec4 permute(vec4 x) {
	return mod289(((x * vec4(34.0)) + vec4(1.0))*x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return vec4(1.79284291400159) - vec4(0.85373472095314) * r;
}

float snoise(vec3 v)
{
	const vec2  C = vec2(1.0 / 6.0, 1.0 / 3.0);
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i = glm::floor(v + glm::dot(v, vec3(C.y)));
	vec3 x0 = v - i + glm::dot(i, vec3(C.x));

	// Other corners
	vec3 g = glm::step(vec3(x0.y, x0.z, x0.x), vec3(x0.x, x0.y, x0.z));
	vec3 l = vec3(1.0) - g;
	vec3 i1 = glm::min(vec3(g.x, g.y, g.z), vec3(l.z, l.x, l.y));
	vec3 i2 = glm::max(vec3(g.x, g.y, g.z), vec3(l.z, l.x, l.y));

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + vec3(C.x);
	vec3 x2 = x0 - i2 + vec3(C.y); // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - vec3(D.y);      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i);
	vec4 p = permute(permute(permute(
		i.z + vec4(0.0, i1.z, i2.z, 1.0))
		+ i.y + vec4(0.0, i1.y, i2.y, 1.0))
		+ i.x + vec4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * vec3(D.w, D.y, D.z) - vec3(D.x, D.z, D.x);

	vec4 j = p - vec4(49.0) * glm::floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = glm::floor(j * ns.z);
	vec4 y_ = glm::floor(j - 7.0f * x_);    // mod(j,N)

	vec4 x = x_ *ns.x + vec4(ns.y);
	vec4 y = y_ *ns.x + vec4(ns.y);
	vec4 h = vec4(1.0) - glm::abs(x) - glm::abs(y);

	vec4 b0 = vec4(x.x, x.y, y.x, y.y);
	vec4 b1 = vec4(x.z, x.w, y.z, y.w);

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = glm::floor(b0)*2.0f + vec4(1.0);
	vec4 s1 = glm::floor(b1)*2.0f + vec4(1.0);
	vec4 sh = -glm::step(h, vec4(0.0));

	vec4 a0 = vec4(b0.x, b0.z, b0.y, b0.w) + vec4(s0.x, s0.z, s0.y, s0.w) * vec4(sh.x, sh.x, sh.y, sh.y);
	vec4 a1 = vec4(b1.x, b1.z, b1.y, b1.w) + vec4(s1.x, s1.z, s1.y, s1.w) * vec4(sh.z, sh.z, sh.w, sh.w);

	vec3 p0 = vec3(a0.x, a0.y, h.x);
	vec3 p1 = vec3(a0.z, a0.w, h.y);
	vec3 p2 = vec3(a1.x, a1.y, h.z);
	vec3 p3 = vec3(a1.z, a1.w, h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(glm::dot(p0, p0), glm::dot(p1, p1), glm::dot(p2, p2), glm::dot(p3, p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = glm::max(vec4(0.6) - vec4(glm::dot(x0, x0), glm::dot(x1, x1), glm::dot(x2, x2), glm::dot(x3, x3)), 0.0);
	m = m * m;
	return 42.0 * glm::dot(m*m, vec4(glm::dot(p0, x0), glm::dot(p1, x1),
		glm::dot(p2, x2), glm::dot(p3, x3)));
}

GLuint CreatePyroclasticVolume(int n, float r)
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned char *data = new unsigned char[n*n*n];
	unsigned char *ptr = data;

	float frequency = 3.0f / n;
	float center = n / 2.0f + 0.5f;

	for (int x = 0; x < n; ++x) {
		for (int y = 0; y < n; ++y) {
			for (int z = 0; z < n; ++z) {
				float dx = center - x;
				float dy = center - y;
				float dz = center - z;

				float off = fabsf(snoise(vec3(x * frequency, y * frequency, z * frequency)));

				float d = sqrtf(dx*dx + dy*dy + dz*dz) / (n);
				bool isFilled = (d - off) < r;
				 *ptr++ = isFilled ? 255 : 0;
			}
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0,
		GL_R32F,
		n, n, n, 0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		data);

	delete[] data;
	return handle;
}

bool load()
{
	if (!shader_intervals.loadAndLinkFromFile("./demo/32volume/intervals") ||
		!shader_raycast.loadAndLinkFromFile("./demo/32volume/raycast"))
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	tex_front.dispose();
	tex_back.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	cube_buffer.create(Mesh::genUnitCube(false, false, true));
	ssquad_buffer.create(Mesh::genScreenSpaceTexQuad());
	cube = Model(cube_buffer);

	tex_front.create(ctx.getWidth(), ctx.getHeight());
	tex_back.create(ctx.getWidth(), ctx.getHeight());
	tex_volume = CreatePyroclasticVolume(128, 0.2f);
	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.2f) * rotateY(ctx.getElapsedTime() * 0.1f);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.beginCustomShader(shader_intervals);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);

	tex_front.begin();
	gfx.setCullState(CullStates::CullCounterClockwiseBack);
	gfx.clear(0x00000000, 1.0);
	cube.draw();
	tex_front.end();

	tex_back.begin();
	gfx.setCullState(CullStates::CullCounterClockwiseFront);
	gfx.clear(0x00000000, 1.0);
	cube.draw();
	tex_back.end();
	gfx.endCustomShader();

	gfx.beginCustomShader(shader_raycast);
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setBlendState(BlendStates::Additive);
	gfx.setCullState(CullStates::CullCounterClockwiseBack);
	tex_front.bindTexture(GL_TEXTURE0);
	gfx.setUniform("texFront", 0);
	tex_back.bindTexture(GL_TEXTURE1);
	gfx.setUniform("texBack", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, tex_volume);
	gfx.setUniform("texVolume", 2);
	ssquad_buffer.draw();
	gfx.endCustomShader();
	gfx.setBlendState(BlendStates::Default);
}