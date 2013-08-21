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

const int primes[] = { 543855877, 431689729, 284866381, 620586229, 935802211, 599684249, 684019249,
500702051, 727501183, 865744549, 988132547, 873958313, 171469003, 318296159, 138157961, 135935707,
480874481, 752454797, 28117273, 323094347, 382719373, 993123553, 881534189, 776528111, 847851293,
497201447, 295567373, 444275963, 695887807, 125496409, 483408553, 695661907, 87703277, 478290947,
412270487, 830931029, 212264167, 831290149, 240976871, 782920781 };

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

void generateTerrainMesh(TriMesh &mesh)
{
	unsigned int i = mesh.getPositionCount();
	mesh.addPosition(-0.5f, -0.5f, 0.5f);
	mesh.addPosition(-0.5f,  0.5f, 0.5f);
	mesh.addPosition( 0.5f,  0.5f, 0.5f);
	mesh.addPosition( 0.5f, -0.5f, 0.5f);
	mesh.addNormal(0.0f, 0.0f, 1.0f);
	mesh.addNormal(0.0f, 0.0f, 1.0f);
	mesh.addNormal(0.0f, 0.0f, 1.0f);
	mesh.addNormal(0.0f, 0.0f, 1.0f);
	mesh.addTriangle(i + 0, i + 1, i + 2);
	mesh.addTriangle(i + 2, i + 3, i + 0);
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

	TriMesh terrainMesh;
	generateTerrainMesh(terrainMesh);

	BufferedMesh terrainBuffer;
	terrainBuffer.create(terrainMesh, program0Layout);

	TriMesh quadMesh;
	quadMesh.addPosition(-0.5f, -0.5f, 0.0f); quadMesh.addTexel(0.0f, 0.0f);
	quadMesh.addPosition(-0.5f,  0.5f, 0.0f); quadMesh.addTexel(0.0f, 1.0f);
	quadMesh.addPosition( 0.5f,  0.5f, 0.0f); quadMesh.addTexel(1.0f, 1.0f);
	quadMesh.addPosition( 0.5f, -0.5f, 0.0f); quadMesh.addTexel(1.0f, 0.0f);
	quadMesh.addTriangle(0, 1, 2);
	quadMesh.addTriangle(2, 3, 0);

	BufferedMesh quadBuffer;
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

	glEnable(GL_TEXTURE_2D);

	Noisemap noisemap;
	noisemap.create(9182);

	GLuint heightmapTexture;
	generateHeightmapTexture(heightmapTexture, noisemap, 512, 512, 10.0f);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 1.0f, 100.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();

		double renderStart = timer.getElapsedTime();

		glClearColor(0.33f, 0.33f, 0.33f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);
		MatrixStack modelMatrix;

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		// Draw terrain
			modelMatrix.push();
			modelMatrix.rotateX(22.0f);
			program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
			program0.uniform(program0Layout.getUniformLoc("white"), 0.0f);
			terrainBuffer.draw();
			modelMatrix.pop();

		program0.unuse();
		viewMatrix.pop();

		// Draw heightmap texture
		program1.use();
		program1.uniform(program1Layout.getUniformLoc("tex"), 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, heightmapTexture);
		quadBuffer.draw();
		glBindTexture(GL_TEXTURE_2D, 0);
		program1.unuse();

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