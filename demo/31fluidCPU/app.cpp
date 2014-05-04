#include "app.h"
#include <common/noise.h>

template <typename T>
struct Field
{
	int width;
	int height;
	T *data;

	void init(int w, int h)
	{
		width  = w;
		height = h;
		data = new T[w * h];
		for (int i = 0; i < w * h; ++i)
			data[i] = T(0);
	}

	void init(Field<T> &copy)
	{
		width  = copy.width;
		height = copy.height;
		data = new T[width * height];
		for (int i = 0; i < width * height; ++i)
			data[i] = copy.data[i];
	}

	void clear()
	{
		for (int i = 0; i < width * height; ++i)
			data[i] = T(0);
	}
	
	void dispose()
	{
		delete[] data;
	}

	T get(int x, int y)
	{
		if (x < 0) x = 0;
		if (x >= width) x = width - 1;
		if (y < 0) y = 0;
		if (y >= height) y = height - 1;

		return data[y * width + x];
	}

	void set(int x, int y, T t)
	{
		data[y * width + x] = t;
	}

	// p is floating in the range [0, N] on both axes
	T sample(const vec2 &p)
	{
		int   xi = int(p.x);
		int   yi = int(p.y);
		float xf = p.x - xi;
		float yf = p.y - yi;

		// Get nearest cells
		T v00 = get(xi, yi);
		T v10 = get(xi + 1, yi);
		T v01 = get(xi, yi + 1);
		T v11 = get(xi + 1, yi + 1);

		// Bilinear lerp
		T va = v00 + (v10 - v00) * xf;
		T vb = v01 + (v11 - v01) * xf;
		return va + (vb - va) * yf;
	}

	T &operator()(int x, int y)
	{
		return data[y * width + x];
	}

	T &operator()(const vec2i &v)
	{
		return data[v.y * width + v.x];
	}
};

template <typename T>
void swap(Field<T> &a, Field<T> &b)
{
	T *temp = a.data;
	a.data = b.data;
	b.data = temp;
}

typedef Field<vec2> Field2f;

struct Slab
{
	Field2f ping;
	Field2f pong;
};

VertexArray vao;
ShaderProgram shader_texture;
MeshBuffer screen_quad;

Texture2D tex_velocity, tex_pressure, tex_dye;

const int GRID_RES = 32;
float DX = 1.0f / GRID_RES;

Field2f velocity;
Field2f pressure;
Field2f divergence;
Field2f dye;

bool load()
{
	if (!shader_texture.loadAndLinkFromFile("./demo/31fluidCPU/texture"))
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	velocity.init(GRID_RES, GRID_RES);
	pressure.init(GRID_RES, GRID_RES);
	divergence.init(GRID_RES, GRID_RES);
	dye.init(GRID_RES, GRID_RES);
	dye(5,  15) = vec2(1.0, 0.0);
	dye(6,  15) = vec2(1.0, 0.0);
	dye(7,  15) = vec2(1.0, 0.0);
	dye(8,  15) = vec2(1.0, 0.0);
	dye(9,  15) = vec2(1.0, 0.0);
	dye(10, 15) = vec2(1.0, 0.0);
	dye(11, 15) = vec2(1.0, 0.0);

	// Boundary conditions
	//for (int i = 0; i < GRID_RES; ++i)
	//{
	//	float x = i	* DX;
	//	velocity.ping(i, 0) = vec2(sin(PI * x));
	//	velocity.ping(0, i) = vec2(sin(PI * x));
	//	velocity.pong(i, 0) = velocity.ping(i, 0);
	//	velocity.pong(0, i) = velocity.ping(i, 0);
	//	velocity.ping(i, GRID_RES - 1) = vec2(sin(PI * x) * exp(-PI));
	//	velocity.pong(i, GRID_RES - 1) = velocity.ping(i, GRID_RES - 1);
	//}
	
	tex_velocity.create(0, GL_RG32F, GRID_RES, GRID_RES, GL_RG, GL_FLOAT, NULL);
	tex_velocity.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_pressure.create(0, GL_RG32F, GRID_RES, GRID_RES, GL_RG, GL_FLOAT, NULL);
	tex_pressure.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_dye.create(0, GL_RG32F, GRID_RES, GRID_RES, GL_RG, GL_FLOAT, NULL);
	tex_dye.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	screen_quad.create(Mesh::genScreenSpaceTexQuad());
}

void jacobi(
	float alpha, float beta,
	Field2f &b,
	Field2f &x0, // Input grid
	Field2f &x1) // Output grid
{
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			// Neighbor samples of x
			vec2 xl = x0(x - 1, y);
			vec2 xr = x0(x + 1, y);
			vec2 xb = x0(x, y - 1);
			vec2 xt = x0(x, y + 1);
			vec2 bc = b(x, y);

			x1(x, y) = (1.0f / beta) * (xl + xr + xb + xt + alpha * bc);
		}
	}
}

vec2 advect(
	vec2i coord,
	float dt,
	Field2f &u,	 // Velocity field
	Field2f &x0) // The quantity to be advected
{
	// Trace velocity field "back in time"
	vec2 coord0 = vec2(coord) - u(coord) * (dt * GRID_RES);

	// Bilinear sample of the quantity at that point
	return x0.sample(coord0);
}

vec2 force(int x, int y)
{
	float dx = (x - GRID_RES / 2);
	float dy = (y - GRID_RES / 2);
	return vec2(0.0, 1.0) * float(exp(-(dx * dx + dy * dy) * 0.05));
}

vec2 gradient(int x, int y, Field2f &f)
{
	vec2 fr = f.get(x + 1, y);
	vec2 fl = f.get(x - 1, y);
	vec2 ft = f.get(x, y + 1);
	vec2 fb = f.get(x, y - 1);
	return vec2(fr.x - fl.x, ft.y - fb.y) * (1.0f / (2.0f * DX));
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	// Temporary velocity field
	Field2f w;
	w.init(GRID_RES, GRID_RES);

	// Advect the velocity field
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			w(x, y) = advect(vec2i(x, y), dt, velocity, velocity);
		}
	}

	Field2f dye_next;
	dye_next.init(GRID_RES, GRID_RES);
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			dye_next(x, y) = advect(vec2i(x, y), dt, velocity, dye);
		}
	}

	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			dye(x, y) = dye_next(x, y);
		}
	}

	// Viscous diffusion
	//Field2f velocity_next;
	//velocity_next.init(velocity);
	//for (int i = 0; i <= 200; ++i)
	//{
	//	jacobi(DX * DX / (NU * dt), 4 + (DX * DX) / (NU * dt), velocity, velocity, velocity_next);
	//	swap(velocity, velocity_next);
	//}

	// Apply forces
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			w(x, y) += force(x, y) * dt;
		}
	}

	// Calculate divergence
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			vec2 xl = w.get(x - 1, y);
			vec2 xr = w.get(x + 1, y);
			vec2 xt = w.get(x, y + 1);
			vec2 xb = w.get(x, y - 1);
			divergence(x, y).x = (xr.x - xl.x + xt.y - xb.y) / (2.0f * DX);
		}
	}

	// Calculate pressure field
	pressure.clear();
	Field2f pressure_next;
	pressure_next.init(pressure);
	for (int i = 0; i <= 40; ++i)
	{
		jacobi(-DX * DX, 4.0f, divergence, pressure, pressure_next);
		swap(pressure, pressure_next);
	}

	// Project by subtracting gradient, and add to velocity field
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			velocity(x, y) = w(x, y) - gradient(x, y, pressure);
		}
	}

	// Boundary conditions
	for (int i = 0; i < GRID_RES; ++i)
	{
		velocity(i, 0) = -velocity(i, 1);
		velocity(i, GRID_RES - 1) = -velocity(i, GRID_RES - 2);
		velocity(0, i) = -velocity(1, i);
		velocity(GRID_RES - 1, i) = -velocity(GRID_RES - 2, i);

		pressure(i, 0) = pressure(i, 1);
		pressure(i, GRID_RES - 1) = pressure(i, GRID_RES - 2);
		pressure(0, i) = pressure(1, i);
		pressure(GRID_RES - 1, i) = pressure(GRID_RES - 2, i);
	}

	// Solution by iteration
	//for (int i = 0; i < 200; ++i)
	//{
	//	jacobi(0.0f, 4.0f, divergence, velocity.ping, velocity.pong);
	//	swap(velocity.ping, velocity.pong);
	//}

	// Update result
	tex_velocity.bind();
	tex_velocity.update(velocity.data, GL_RG, GL_FLOAT);
	tex_velocity.unbind();
	tex_pressure.bind();
	tex_pressure.update(pressure.data, GL_RG, GL_FLOAT);
	tex_pressure.unbind();
	tex_dye.bind();
	tex_dye.update(dye.data, GL_RG, GL_FLOAT);
	tex_dye.unbind();
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	tex_pressure.bind(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	screen_quad.draw();
}