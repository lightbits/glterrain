/*
OpenGL heightmap terrain.
Generates a heightmap using 2D noise, and generates a triangle mesh.
It uses fractional brownian motion functions (fBm), and bilinear interpolation
through a grid of random values.

http://www.scratchapixel.com/lessons/3d-advanced-lessons/noise-part-1/creating-a-simple-2d-noise/
http://www.scratchapixel.com/lessons/3d-advanced-lessons/interpolation/
http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl?rq=1
http://stackoverflow.com/questions/6503257/per-vertex-normals-from-perlin-noise
http://www.iquilezles.org/www/articles/sfrand/sfrand.htm
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

// Returns a random floating point between 0 and 1
float noise2f(int x, int y)
{
	int n = x + y * 57;
	n = (n<<13) ^ n;
    return 0.5f * (1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) + 0.5f;
}

float smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }

// Returns the smoothed bilinear interpolation of the noise values in the square containing (x, y)
float snoise(float x, float y)
{
	float xi = floor(x);
	float yi = floor(y);
	float xf = x - xi;
	float yf = y - yi;

	float h00 = noise2f(xi, yi);
	float h10 = noise2f(xi + 1, yi);
	float h01 = noise2f(xi, yi + 1);
	float h11 = noise2f(xi + 1, yi + 1);

	float u = smoothstep(xf);
	float v = smoothstep(yf);

	float x0 = lerp(h00, h10, u);
	float x1 = lerp(h01, h11, u);

	return lerp(x0, x1, v);
}

// Computes a fractal-based noise value at the point (x, y) by summing noise
// of different frequencies and amplitudes
float fBm(float x, float y)
{
	float amplitude = 1.0f; // 2.0f
	float frequency = 1.0f;
	float noise = 0.0f;
	float max = 0.0f;
	for(int i = 0; i < 5; ++i)
	{
		max += amplitude;
		noise += snoise(x * frequency, y * frequency) * amplitude;
		amplitude *= 0.5f; // 0.43f
		frequency *= 2.0f;
	}

	return noise;
	// Normalize the result
	//return noise / max;
}

// Generates a terrain mesh using the noisemap as source for a heightmap
void generateTerrainMesh(TriMesh &mesh, int resX, int resY, int sampleScale, float meshScale)
{
	std::vector<float> heights(resX * resY);
	for(int i = 0; i < heights.size(); ++i)
	{
		int xi = i % resX;
		int yi = i / resX;
		float xf = sampleScale * xi / float(resX);
		float yf = sampleScale * yi / float(resY);
		heights[i] = fBm(xf, yf);
	}

	mesh.clear();
	for(int y = 0; y < resY - 1; ++y)
	{
		for(int x = 0; x < resX - 1; ++x)
		{
			float x0 = sampleScale * x / float(resX);
			float x1 = sampleScale * (x + 1) / float(resX);
			float y0 = sampleScale * y / float(resY);
			float y1 = sampleScale * (y + 1) / float(resY);
			float h00 = heights[y * resX + x];
			float h10 = heights[y * resX + x + 1];
			float h01 = heights[(y + 1) * resX + x];
			float h11 = heights[(y + 1) * resX + x + 1];

			float mx0 = (x / float(resX) - 0.5f) * meshScale;
			float mx1 = ((x + 1) / float(resX) - 0.5f) * meshScale;
			float my0 = (y / float(resY) - 0.5f) * meshScale;
			float my1 = ((y + 1) / float(resY) - 0.5f) * meshScale;
			unsigned int i = mesh.getPositionCount();
			mesh.addPosition(mx0, h00, my0);
			mesh.addPosition(mx1, h10, my0);
			mesh.addPosition(mx1, h11, my1);
			mesh.addTriangle(i + 0, i + 1, i + 2);

			mesh.addPosition(mx1, h11, my1);
			mesh.addPosition(mx0, h01, my1);
			mesh.addPosition(mx0, h00, my0);
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

	std::cout<<"Generated terrain ("<<mesh.positions.size()<<" vertices)"<<std::endl;
}

void generateHeightmapTexture(GLuint &texture, unsigned int width, unsigned int height, float scale)
{
	std::vector<GLfloat> pixels(width*height*3);
	for(unsigned int y = 0; y < height; ++y)
	{
		for(unsigned int x = 0; x < width; ++x)
		{
			float s = fBm(scale * x / float(width), scale * y / float(height));
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
	if(!gl::createContext("Terrain", 300, 100, 720, 480, 24, 8, 8, false))
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

	GLuint heightmapTexture;
	generateHeightmapTexture(heightmapTexture, 96, 96, 4.0f);

	TriMesh terrainMesh;
	BufferedMesh terrainBuffer;
	generateTerrainMesh(terrainMesh, 96, 96, 24.0f, 24.0f);
	terrainBuffer.create(terrainMesh, program0Layout);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 720.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();

		update(timer.getElapsedTime());

		double renderStart = timer.getElapsedTime();
		glClearColor(0.55f, 0.45f, 0.45f, 1.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, -0.7f, -4.0f + zoom);
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