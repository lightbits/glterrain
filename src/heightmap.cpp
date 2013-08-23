/*
OpenGL heightmap terrain.
Generates a heightmap using 2D noise, and generates a triangle mesh.
*/
#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <graphics/opengl.h>
#include <graphics/color.h>
#include <graphics/texture.h>
#include <graphics/trimesh.h>
#include <graphics/program.h>
#include <graphics/bufferobject.h>
#include <graphics/vertexformat.h>
#include <graphics/spritebatch.h>
#include <graphics/bufferedmesh.h>
using namespace graphics;

void shutdown(const char *error = "")
{
	if(error != "")
	{
		std::cerr<<error<<std::endl;
		std::cin.get();
	}
	gl::destroyContext();
	exit(error != "" ? EXIT_FAILURE : EXIT_SUCCESS);
}

// Returns a smoothed value between 0 and 1, for t between 0 and 1
inline float smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }

class Noisemap
{
public:
	Noisemap() : grid(NULL) { }
	~Noisemap() { if(grid) delete[] grid; }

	void create(unsigned int seed = 2011)
	{
		grid = new float[size * size];
		srand(seed);
		for(unsigned int i = 0; i < size * size; ++i)
			grid[i] = rand() / float(RAND_MAX);
	}

	float eval(float x, float y) const
	{
		// The grid-snapped coordinates
		int xi = floor(x);
		int yi = floor(y);

		// The fractional parts of x and y
		float xf = x - xi;
		float yf = y - yi;

		// Get the grid-snapped coordinates of the square containing (x, y),
		// taking the modulo of the grid-size if we go outside.
		// (Note that a & b = a mod b when (b+1) is a power of two)
		int rx0 = xi & (size - 1);
		int ry0 = yi & (size - 1);
		int rx1 = (xi + 1) & (size - 1);
		int ry1 = (yi + 1) & (size - 1);

		// Retrieve the noise values at the square corners
		const float &c00 = grid[ry0 * size + rx0]; // top-left
		const float &c10 = grid[ry0 * size + rx1]; // top-right
		const float &c01 = grid[ry1 * size + rx0]; // bottom-left
		const float &c11 = grid[ry1 * size + rx1]; // bottom-right

		// Remap the fractional parts for smoothing purposes
		float tx = smoothstep(xf);
		float ty = smoothstep(yf);

		// Linearly interpolate values along the x-axis
		float nx0 = lerp(c00, c10, tx);
		float nx1 = lerp(c01, c11, tx);

		// Linearly interpolate on y-axis and return the final value
		return lerp(nx0, nx1, ty);
	}
private:
	float *grid;
	static const unsigned int size = 256; // Must be power of two (due to modulo operations)
};

// Computes a fractal-based noise value at the point (x, y),
// by summing contributions of noise layers with double frequency
// and half amplitude of the previous
// http://www.scratchapixel.com/lessons/3d-advanced-lessons/noise-part-1/pattern-examples/
float fractalNoise(const Noisemap &noisemap, float x, float y)
{
	static const unsigned int layerCount = 5;
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float noise = 0.0f;
	float max = 0.0f;
	for(unsigned int i = 0; i < layerCount; ++i)
	{
		max += amplitude;
		noise += noisemap.eval(frequency * x, frequency * y) * amplitude;
		amplitude *= 0.5f;
		frequency *= 2.0f;
	}

	// Normalize the result
	return noise / max;
}

// Generates a terrain mesh using the noisemap as source for a heightmap
void generateTerrainMesh(TriMesh &mesh, const Noisemap &noisemap, unsigned int resX, unsigned int resY, float scale)
{
	std::vector<float> heights(resX * resY);
	for(unsigned int i = 0; i < heights.size(); ++i)
	{
		unsigned int xi = i % resX;
		unsigned int yi = i / resX;
		float xf = scale * xi / float(resX);
		float yf = scale * yi / float(resY);
		heights[i] = fractalNoise(noisemap, xf, yf);
	}	

	mesh.clear();
	for(int y = 0; y < resY - 1; ++y)
	{
		for(int x = 0; x < resX - 1; ++x)
		{
			float x0 = scale * x / float(resX);
			float x1 = scale * (x + 1) / float(resX);
			float y0 = scale * y / float(resY);
			float y1 = scale * (y + 1) / float(resY);
			float h00 = heights[y * resX + x];
			float h10 = heights[y * resX + x + 1];
			float h01 = heights[(y + 1) * resX + x];
			float h11 = heights[(y + 1) * resX + x + 1];
			float ox = scale / 2.0f;
			float oy = scale / 2.0f;
			unsigned int i = mesh.getPositionCount();
			mesh.addPosition(x0 - ox, h00, y0 - oy);
			mesh.addPosition(x1 - ox, h10, y0 - oy);
			mesh.addPosition(x1 - ox, h11, y1 - oy);
			mesh.addTriangle(i + 0, i + 1, i + 2);

			mesh.addPosition(x1 - ox, h11, y1 - oy);
			mesh.addPosition(x0 - ox, h01, y1 - oy);
			mesh.addPosition(x0 - ox, h00, y0 - oy);
			mesh.addTriangle(i + 3, i + 4, i + 5);
		}
	}

	// Compute normals
	for(int i = 0; i < mesh.indices.size(); i += 3)
	{
		vec3 v0 = mesh.positions[mesh.indices[i + 0]];
		vec3 v1 = mesh.positions[mesh.indices[i + 1]];
		vec3 v2 = mesh.positions[mesh.indices[i + 2]];
		vec3 n = glm::normalize(glm::cross((v2 - v0), (v1 - v0)));
		mesh.addNormal(n);
		mesh.addNormal(n);
		mesh.addNormal(n);
	}
}

void generateHeightmapTexture(GLuint &texture, const Noisemap &noisemap, unsigned int width, unsigned int height, float scale)
{
	std::vector<GLfloat> pixels(width*height*3);
	for(unsigned int y = 0; y < height; ++y)
	{
		for(unsigned int x = 0; x < width; ++x)
		{
			float s = fractalNoise(noisemap, scale * x / float(width), scale * y / float(height));
			pixels[3 * (y * width + x) + 0] = s;
			pixels[3 * (y * width + x) + 1] = s;
			pixels[3 * (y * width + x) + 2] = s;
		}
	}

	texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, &pixels[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

double time0 = 0.0;
int lastMouseX = 320;
int lastMouseY = 240;
float rotationSpeedX = 0.0f;
float rotationSpeedY = 0.0f;
float rotationX = 0.0f;
float rotationY = 0.0f;
float zoom = 0.0f;
bool keydown = false;
bool wireframe = true;
void update(double time)
{
	double dt = time - time0;

	if(glfwGetKey(GLFW_KEY_SPACE) && !keydown)
	{
		wireframe = !wireframe;
		keydown = true;
	}
	else if(!glfwGetKey(GLFW_KEY_SPACE))
	{
		keydown = false;
	}

	int mouseX, mouseY;
	glfwGetMousePos(&mouseX, &mouseY);
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
	{
		int dx = mouseX - lastMouseX;
		int dy = mouseY - lastMouseY;
		rotationSpeedY += float(dx) * 0.0005f;
		rotationSpeedX += float(dy) * 0.0005f;
	}

	rotationSpeedX = 0.95f * rotationSpeedX;
	rotationSpeedY = 0.95f * rotationSpeedY;
	rotationX += rotationSpeedX;
	rotationY += rotationSpeedY;
	lastMouseX = mouseX;
	lastMouseY = mouseY;
	zoom = float(glfwGetMouseWheel()) * 0.05f;

	time0 = time;
}

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 640, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	std::string 
		terrain_vert_src, 
		terrain_frag_src,
		texture_vert_src,
		texture_frag_src;

	if(!readFile("data/shaders/terrain.vert", terrain_vert_src) ||
		!readFile("data/shaders/terrain.frag", terrain_frag_src) ||
		!readFile("data/shaders/texture.vert", texture_vert_src) ||
		!readFile("data/shaders/texture.frag", texture_frag_src))
		shutdown("Failed to load resources");

	Program program0;
	program0.compile(terrain_vert_src, terrain_frag_src);

	ShaderLayout program0Layout;
	program0Layout.attribs["position"] = program0.getAttribLocation("position");
	program0Layout.attribs["normal"] = program0.getAttribLocation("normal");
	program0Layout.uniforms["projection"] = program0.getUniformLocation("projection");
	program0Layout.uniforms["model"] = program0.getUniformLocation("model");
	program0Layout.uniforms["view"] = program0.getUniformLocation("view");
	program0Layout.uniforms["white"] = program0.getUniformLocation("white");

	Program program1;
	program1.compile(texture_vert_src, texture_frag_src);

	ShaderLayout program1Layout;
	program1Layout.attribs["position"] = program1.getAttribLocation("position");
	program1Layout.attribs["texel"] = program1.getAttribLocation("texel");

	// A quad mesh to display the noisemap texture
	TriMesh quadMesh;
	BufferedMesh quadBuffer;
	quadMesh.addPosition(-0.5f, -0.5f, 0.0f); quadMesh.addTexel(0.0f, 0.0f);
	quadMesh.addPosition(-0.5f,  0.5f, 0.0f); quadMesh.addTexel(0.0f, 1.0f);
	quadMesh.addPosition( 0.5f,  0.5f, 0.0f); quadMesh.addTexel(1.0f, 1.0f);
	quadMesh.addPosition( 0.5f, -0.5f, 0.0f); quadMesh.addTexel(1.0f, 0.0f);
	quadMesh.addTriangle(0, 1, 2);
	quadMesh.addTriangle(2, 3, 0);
	quadBuffer.create(quadMesh, program1Layout);

	// Whatever-array-object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	glFrontFace(GL_CW);
	glEnable(GL_TEXTURE_2D);

	Noisemap noisemap;
	noisemap.create(9182);

	GLuint heightmapTexture;
	generateHeightmapTexture(heightmapTexture, noisemap, 512, 512, 10.0f);

	TriMesh terrainMesh;
	BufferedMesh terrainBuffer;
	generateTerrainMesh(terrainMesh, noisemap, 48, 48, 8.0f);
	terrainBuffer.create(terrainMesh, program0Layout);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.1f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();

		update(timer.getElapsedTime());

		double renderStart = timer.getElapsedTime();
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -4.0f + zoom);
		MatrixStack modelMatrix;

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		// Draw terrain
			modelMatrix.push();
			modelMatrix.rotateX(rotationX * 180.0f / 3.1415f);
			modelMatrix.rotateY(rotationY * 180.0f / 3.1415f);
			program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
			program0.uniform(program0Layout.getUniformLoc("white"), 0.0f);
			terrainBuffer.draw();

			if(wireframe)
			{
				program0.uniform(program0Layout.getUniformLoc("white"), 1.0f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				terrainBuffer.draw();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			
			modelMatrix.pop();

			
		program0.unuse();
		viewMatrix.pop();

		
		//program1.use();

		//// Draw heightmap texture
		//	program1.uniform(program1Layout.getUniformLoc("tex"), 0);
		//	glActiveTexture(GL_TEXTURE0 + 0);
		//	glBindTexture(GL_TEXTURE_2D, heightmapTexture);
		//	quadBuffer.draw();
		//	glBindTexture(GL_TEXTURE_2D, 0);

		//program1.unuse();

		glfwSwapBuffers();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			glfwSleep(0.013 - renderTime);

		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<gl::getErrorMessage(error)<<"...";
			std::cin.get();
			glfwCloseWindow();
		}
	}

	program0.dispose();
	terrainBuffer.dispose();
	shutdown();
}