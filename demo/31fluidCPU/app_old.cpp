#include "app.h"
#include <common/noise.h>

VertexArray vao;
ShaderProgram shader_texture;
MeshBuffer screen_quad;

Texture2D tex_velocity;
Texture2D tex_pressure;

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

	T sample(const vec2 &texel)
	{
		int   xi = int(texel.x);
		int   yi = int(texel.y);
		float xf = texel.x - xi;
		float yf = texel.y - yi;

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
};

typedef Field<vec2> Field2f;

const int   GRID_RES = 20;
const float DX       = 1.0f / GRID_RES;
const float NU		 = 1.5f; // Viscosity
int inputIndex       = 0;
int outputIndex      = 1;

struct Slab
{
	Field2f Ping;
	Field2f Pong;
};

void swapSurfaces(Slab &slab)
{
	vec2 *temp = slab.Ping.data;
	slab.Ping.data = slab.Pong.data;
	slab.Pong.data = temp;
}

Slab velocity, pressure;
Field2f div_field;

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

	//int tex_res_x = ctx.getWidth();
	//int tex_res_y = ctx.getHeight();

	velocity.Ping.init(GRID_RES, GRID_RES);
	velocity.Pong.init(GRID_RES, GRID_RES);
	pressure.Ping.init(GRID_RES, GRID_RES);
	pressure.Pong.init(GRID_RES, GRID_RES);
	div_field.init(GRID_RES, GRID_RES);
	
	tex_velocity.create(0, GL_RGB32F, GRID_RES, GRID_RES, GL_RG, GL_FLOAT, NULL);
	tex_pressure.create(0, GL_RGB32F, GRID_RES, GRID_RES, GL_RG, GL_FLOAT, NULL);

	tex_velocity.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_pressure.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	screen_quad.create(Mesh::genScreenSpaceTexQuad());

	pressure.Ping.set(7, 5, vec2(10.0f, 0.0));
	pressure.Ping.set(6, 5, vec2(10.0f, 0.0));
	pressure.Ping.set(5, 5, vec2(10.0f, 0.0));
	pressure.Pong.set(7, 5, vec2(10.0f, 0.0));
	pressure.Pong.set(6, 5, vec2(10.0f, 0.0));
	pressure.Pong.set(5, 5, vec2(10.0f, 0.0));
}

float divergence(int x, int y, Field2f &f)
{
	float ul = f.get(x - 1, y).x;
	float ur = f.get(x + 1, y).x;
	float vt = f.get(x, y + 1).y;
	float vb = f.get(x, y - 1).y;

	return (ur - ul + vt - vb) / (2.0f * DX);
}

vec2 gradient(int x, int y, Field2f &f)
{
	float ul = f.get(x - 1, y).x;
	float ur = f.get(x + 1, y).x;
	float vt = f.get(x, y + 1).y;
	float vb = f.get(x, y - 1).y;

	return vec2(ur - ul, vt - vb) * (1.0f / (2.0f * DX));
}

/*
Iterates using the formula
	x_i_j_k+1 = (1/beta) (x_i-1_j_k + ... + x_i_j-1_k + alpha * b_i_j)

				equation:
parameter		diffusion			pressure
-------------------------------------------
x				velocity field		pressure field		
b				velocity field		div(w)
alpha			(dx)^2 / (nu * dt)	-(dx)^2
beta			4 + alpha			4

where dx = 1 / GRID_RES
	  nu = viscosity
	  dt = timestep
	  w  = the divergent temporary velocity field
*/
void jacobi(
	int x, int y, // Grid coordinates 
	float alpha,  // Iteration parameters
	float beta,
	Field2f &b,
	Field2f &x0, // The previous value for x (input)
	Field2f &x1) // The next value for x (output)
{
	// Neighbor samples of x
	vec2 xl = x0.get(x - 1, y);
	vec2 xr = x0.get(x + 1, y);
	vec2 xb = x0.get(x, y - 1);
	vec2 xt = x0.get(x, y + 1);

	// b sample from current grid cell
	vec2 bc = b.get(x, y);

	vec2 xn = (1.0f / beta) * (xl + xr + xb + xt + alpha * bc);
	x1.set(x, y, xn);
}

void jacobi(
	float alpha,  // Iteration parameters
	float beta,
	Field2f &b,
	Field2f &x0, // The previous value for x (input)
	Field2f &x1)
{
	for (int y = 1; y < GRID_RES - 1; ++y)
	for (int x = 1; x < GRID_RES - 1; ++x)
		jacobi(x, y, alpha, beta, b, x0, x1);
}

void advect(int x, int y, float dt, Field2f &u, Field2f &x0, Field2f &x1)
{
	// Follow the velocity field back in time (in grid space)
	vec2 coord = vec2(float(x), float(y));
	vec2 coord0 = coord - dt * GRID_RES * u.get(x, y);

	// Interpolate and write to the output fragment
	x1.set(x, y, x0.sample(coord0));
}

void advect(float dt, Field2f &u, Field2f &x0, Field2f &x1)
{
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES  - 1; ++x)
		{
			advect(x, y, dt, u, x0, x1);
		}
	}
}

void applyForce(int x, int y, Field2f &u)
{
	vec2 c = u.get(GRID_RES / 2, GRID_RES / 2);
	u.set(GRID_RES / 2 + 1, GRID_RES / 2, c + vec2(0.0, 0.5));
	u.set(GRID_RES / 2 + 2, GRID_RES / 2, c + vec2(0.0, 0.5));
	u.set(GRID_RES / 2 + 3, GRID_RES / 2, c + vec2(0.0, 0.5));
}

void applyForce(Field2f &u)
{
	for (int y = 1; y < GRID_RES - 1; ++y)
	for (int x = 1; x < GRID_RES - 1; ++x)
		applyForce(x, y, u);
}

//void doFragment(int x, int y, float dt)
//{
//	const float dx = 1.0f / GRID_RES;
//
//	// Advection of velocity field
//	advect(x, y, dt, 
//		velocity_field[inputIndex], 
//		velocity_field[inputIndex], 
//		velocity_field[outputIndex]);
//
//	// Viscous diffusion
//	float alpha = (DX * DX) / (NU * dt);
//	float beta = 4 + alpha;
//	for (int i = 0; i < 40; ++i)
//		jacobi(x, y, alpha, beta, 
//			velocity_field[outputIndex], 
//			velocity_field[outputIndex], 
//			velocity_field[outputIndex]);  
//
//	// Force application
//	applyForce(x, y, velocity_field[outputIndex]);
//
//	// Solve the Possion-pressure equation for p
//	float divw = divergence(x, y, velocity_field[outputIndex]);
//	temp_field.set(x, y, vec2(divw, 0.0f));
//	jacobi(x, y, - DX * DX, 4, temp_field, pressure_field[inputIndex], pressure_field[outputIndex]);
//	for (int i = 0; i < 40; ++i)
//		jacobi(x, y, - DX * DX, 4, 
//			temp_field, 
//			pressure_field[outputIndex], 
//			pressure_field[outputIndex]); 
//
//	// After solving for p we can project onto the divergence-free component
//	// of the new velocity field by subtracting grad(p)
//	vec2 gradp = gradient(x, y, pressure_field[outputIndex]);
//	vec2 u = velocity_field[outputIndex].get(x, y);
//	velocity_field[outputIndex].set(x, y, u - gradp);
//}

void doBoundary(vec2i coord, vec2i offset)
{
	vec2i coord1 = coord + offset;
	vec2 u_inner = velocity.Ping.get(coord1.x, coord1.y);
	vec2 p_inner = pressure.Ping.get(coord1.x, coord1.y);
	velocity.Pong.set(coord.x, coord.y, -u_inner);
	pressure.Pong.set(coord.x, coord.y,  p_inner);
}

void subtractGradient(Field2f &u0, Field2f &p, Field2f &u1)
{
	for (int y = 1; y < GRID_RES - 1; ++y)
	{
		for (int x = 1; x < GRID_RES - 1; ++x)
		{
			vec2 gradp = gradient(x, y, p);
			vec2 u = u1.get(x, y);
			u1.set(x, y, u - gradp);
		}
	}
}

//void update(Renderer &gfx, Context &ctx, double dt)
//{
//	const float dx = 1.0f / GRID_RES;
//
//	advect(dt, velocity.Ping, velocity.Ping, velocity.Pong);
//	swapSurfaces(velocity);
//
//	// Viscous diffusion by Jacobi iteration
//	float alpha = (DX * DX) / (NU * dt);
//	float beta = 4 + alpha;
//	for (int i = 0; i < 40; ++i)
//	{
//		jacobi(alpha, beta, velocity.Ping, velocity.Ping, velocity.Pong);
//		swapSurfaces(velocity);
//	}
//
//	applyForce(velocity.Ping);
//	swapSurfaces(velocity);
//
//	// Calculate the divergence of the newly updated velocity field
//	for (int y = 1; y < GRID_RES - 1; ++y)
//	for (int x = 1; x < GRID_RES - 1; ++x)
//		div_field.set(x, y, vec2(divergence(x, y, velocity.Pong), 0.0f));
//	pressure.Ping.clear();
//
//	// Solve the Possion-pressure equation for p
//	for (int i = 0; i < 40; ++i)
//	{
//		jacobi(-DX * DX, 4, div_field, pressure.Ping, pressure.Pong);
//		swapSurfaces(pressure);
//	}
//
//	// After solving for p we can project onto the divergence-free component
//	// of the new velocity field by subtracting grad(p)
//	subtractGradient(velocity.Ping, pressure.Ping, velocity.Pong);
//	//swapSurfaces(velocity);
//
//	// Boundary conditions
//	for (int i = 0; i < GRID_RES; ++i)
//	{
//		doBoundary(vec2i(0, i), vec2(1, 0)); // Left wall
//		doBoundary(vec2i(i, 0), vec2(0, 1)); // Bottom wall
//		doBoundary(vec2i(GRID_RES - 1, i), vec2(-1, 0)); // Right wall
//		doBoundary(vec2i(i, GRID_RES - 1), vec2(0, -1)); // Top wall
//	}
//
//	tex_velocity.bind();
//	tex_velocity.update(velocity.Pong.data, GL_RG, GL_FLOAT);
//	tex_velocity.unbind();
//
//	tex_pressure.bind();
//	tex_pressure.update(pressure.Pong.data, GL_RG, GL_FLOAT);
//	tex_pressure.unbind();
//}


void update(Renderer &gfx, Context &ctx, double dt)
{


	tex_velocity.bind();
	tex_velocity.update(velocity.Pong.data, GL_RG, GL_FLOAT);
	tex_velocity.unbind();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	tex_velocity.bind(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	screen_quad.draw();
}