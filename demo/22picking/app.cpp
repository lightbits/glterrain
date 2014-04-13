#include "app.h"

MeshBuffer sphere_buffer;

struct Sphere
{
	Sphere() : position(0.0f), radius(0.0f), model() { }

	Sphere(vec3 p, float r, Model m) : position(p), radius(r), model(m) 
	{
		model.translate(p);
		model.scale(r);
	}

	vec3 position;
	float radius;
	Model model;
};

bool intersect(const Sphere &sphere, const vec3 &p)
{
	vec3 d = p - sphere.position;
	return glm::dot(d, d) <= sphere.radius * sphere.radius;
}

const int NUM_SPHERES = 3;
Sphere spheres[NUM_SPHERES];

mat4
	view,
	projection;

ShaderProgram
	shader_color;

vec3 ray_origin(0.0f, 0.0f, 0.0f);
vec3 ray_direction(0.0f, 0.0f, 0.0f);

int selected_object = -1;
const float z_near = 1.0f;
const float z_far  = 10.0f;
const float fov_y  = 45.0f;
int width, height;

bool load()
{
	if (!shader_color.loadFromFile("./demo/22picking/color"))
		return false;
	if (!shader_color.linkAndCheckStatus())
		return false;
	return true;
}

void free()
{

}

Mesh generateUnitSphere(int t_samples, int s_samples)
{
	Mesh mesh;
	float dtheta = TWO_PI / float(t_samples);
	float dphi = PI / float(s_samples);
	for (int t = 0; t < t_samples; ++t)
	{
		for (int s = 0; s < s_samples; ++s)
		{
			float theta = t * dtheta;
			float phi = s * dphi;

			float r0 = sin(phi);
			float r1 = sin(phi + dphi);

			vec3 v00(r0 * cos(theta), cos(phi), r0 * sin(theta));
			vec3 v10(r0 * cos(theta + dtheta), cos(phi), r0 * sin(theta + dtheta));
			vec3 v01(r1 * cos(theta), cos(phi + dphi), r1 * sin(theta));
			vec3 v11(r1 * cos(theta + dtheta), cos(phi + dphi), r1 * sin(theta + dtheta));
			
			vec3 vertices[] = { v00, v01, v11, v10 };
			uint32 j = mesh.getPositionCount();
			uint32 indices[] = { j, j + 1, j + 2, j + 2, j + 3, j };
			mesh.addIndices(indices, 6);
			mesh.addPositions(vertices, 4);
		}
	}
	return mesh;
}

void init(Renderer &gfx, Context &ctx)
{
	Mesh sphere = generateUnitSphere(32, 32);
	sphere_buffer = MeshBuffer(sphere);

	spheres[0] = Sphere(vec3(0.0f, 0.0f, -0.8f), 0.6f, Model(sphere_buffer));
	spheres[1] = Sphere(vec3(0.8f, 0.11f, -0.3f), 0.5f, Model(sphere_buffer));
	spheres[2] = Sphere(vec3(-0.9f, 0.05f, -1.3f), 0.8f, Model(sphere_buffer));

	view = transform::translate(0.0f, 0.0f, -2.0f);
	projection = transform::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), z_near, z_far);
	width = ctx.getWidth();
	height = ctx.getHeight();
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setClearDepth(1.0);
	gfx.setClearColor(Color::fromHex(0x7f7f7fff));
	gfx.clearColorAndDepth();

	mat4 actual_view = transform::translate(0.0f, 0.0f, -4.4f) * transform::rotateX(-0.3f) * transform::rotateY(0.5f);

	gfx.beginCustomShader(shader_color);
	gfx.setUniform("view", actual_view);
	gfx.setUniform("projection", projection);

	vec4 colors[NUM_SPHERES] = {
		vec4(1.0, 0.8, 0.8, 1.0),
		vec4(0.8, 1.0, 1.0, 1.0),
		vec4(0.8, 0.8, 1.0, 1.0)
	};
	for (int i = 0; i < NUM_SPHERES; ++i)
	{
		if (i == selected_object)
			gfx.setUniform("color", vec4(1.0, 0.2, 0.2, 1.0));
		else
			gfx.setUniform("color", colors[i]);
		spheres[i].model.draw();
	}

	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	vec3 v0(ray_origin);
	vec3 v1(ray_origin + 10.0f * ray_direction);
	float data[] = { 
		v0.x, v0.y, v0.z,  
		v1.x, v1.y, v1.z
	};
	vbo.bufferData(sizeof(data), data);
	gfx.setAttributefv("position", 3, 3, 0);
	gfx.setUniform("model", mat4(1.0));
	gfx.setUniform("color", vec4(1.0, 0.2, 0.2, 1.0));
	glDrawArrays(GL_LINES, 0, 2);
	vbo.unbind();
	vbo.dispose();

	gfx.endCustomShader();
}

bool raycast(int *index, vec3 ro, vec3 rd)
{
	*index = -1;
	float t = 0.0f;
	float t_max = 10.0f;
	while (t < t_max)
	{
		vec3 p = ro + t * rd;
		for (int i = 0; i < NUM_SPHERES; ++i)
		{
			if (intersect(spheres[i], p))
			{
				*index = i;
				return true;
			}
		}
		t += 0.005f;
	}
	return false;
}

void mousePressed(int button, int x, int y)
{
	if (button != SDL_BUTTON_LEFT)
		return;

	// project 2D window coordinate -> point on near plane
	// generate ray direction (-normalize(projected point))
	// transform by inverse view matrix to get ray direction 
	// in world space

	// convert to 3d normalized coordinates
	float x_ndc = -1.0f + (2.0f * x) / width;
	float y_ndc = +1.0f - (2.0f * y) / height;

	/*
	float x_clip = x_ndc * z_near;
	float y_clip = y_ndc * z_near;
	vec4 dir = glm::inverse(projection) * vec4(x_clip, y_clip, z_near, z_near);
	*/
	// Since we normalize, we might as well divide by z_near to get
	vec4 dir = glm::inverse(projection) * vec4(x_ndc, y_ndc, 1.0, 1.0);

	// convert to view coordinates
	//float f = tan(fov_y / 2.0f);
	//float x_view = x_ndc * z_near * f * (float)width / height;
	//float y_view = y_ndc * z_near * f;
	//float z_view = -z_near;

	// ray direction is from camera center (0, 0, 0) to
	// the projected point. (w = 0 because the vector has no position).
	//vec4 dir(x_view, y_view, z_view, 0.0f);

	// compute ray direction in world coordinates
	dir = glm::inverse(view) * dir;
	dir = glm::normalize(dir);

	// origin (in view space) is at camera
	// note that we don't really need to invert
	// the view matrix here, as this vector is only
	// affected by translations.
	vec4 origin(0.0f, 0.0f, 0.0f, 1.0f);
	origin = glm::inverse(view) * origin;

	ray_origin = vec3(origin);
	ray_direction = vec3(dir);

	int index = -1;
	if (raycast(&index, vec3(origin), vec3(dir)))
		selected_object = index;

	cout << "click (" << x << ", " << y << ")\n";
	cout << "index " << index << '\n';
}