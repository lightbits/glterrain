/*
Keywords: deformation, jacobian, waves, vertex shader
http://http.developer.nvidia.com/GPUGems/gpugems_ch42.html
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

#include <camera/camera.h>

#include <gl/bufferobject.h>
#include <gl/vertexarray.h>
#include <gl/vertexformat.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <graphics/spritebatch.h>
#include <graphics/color.h>
#include <graphics/mesh.h>
#include <graphics/renderer.h>
#include <graphics/model.h>
#include <graphics/rendertexture.h>
#include <app/glcontext.h>

GLContext context;
Renderer renderer;

const int windowWidth = 640;
const int windowHeight = 480;

ShaderProgram 
	colorShader,
	waterShader;

Texture waterNormals;

bool initialize()
{
	if (!context.create(VideoMode(windowWidth, windowHeight, 0, 0, 4, 3, 1, false), "Water", true, true))
		return false;

	renderer.init();
	return true;
}

bool loadShaders()
{
	if(!colorShader.loadFromFile("demo/17water/color") ||
	   !waterShader.loadFromFile("demo/17water/water"))
	   return false;

	if(!colorShader.linkAndCheckStatus() ||
	   !waterShader.linkAndCheckStatus())
	   return false;

	return true;
}

bool loadContent()
{
	if(!loadShaders())
		return false;

	if(!waterNormals.loadFromFile("data/textures/waternormal.jpg"))
		return false;

	return true;
}

float yAxisRotation = 0.0f;
float xAxisRotation = -0.78f;
bool reloading = false;

void update(double time, double dt)
{
	if(glfwGetKey(GLFW_KEY_LEFT)) yAxisRotation -= 0.78f * dt;
	if(glfwGetKey(GLFW_KEY_RIGHT)) yAxisRotation += 0.78f * dt;
	if(glfwGetKey(GLFW_KEY_UP)) xAxisRotation += 0.78f * dt;
	if(glfwGetKey(GLFW_KEY_DOWN)) xAxisRotation -= 0.78f * dt;

	if(glfwGetKey('R') && !reloading)
	{
		reloading = true;
		if(!loadShaders())
			shutdown("Failed to load shaders");
	}
	else if(!glfwGetKey('R') && reloading)
	{
		reloading = false;
	}
}

void run()
{
	if(!initialize())
		shutdown("Failed to initialize");

	if(!loadContent())
		shutdown("Failed to load resources");	

	Mesh cubeMesh = Mesh::genUnitColoredCube();
	MeshBuffer cubeBuffer(cubeMesh);
	Model cube(cubeBuffer);

	Mesh waterMesh = Mesh::genUnitColoredPlane(Color(0.57f, 0.63f, 0.98f));
	MeshBuffer waterBuffer(waterMesh);
	Model water(waterBuffer);

	BufferObject quadVbo;
	float quadVertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		+1.0f, -1.0f, 1.0f, 0.0f,
		+1.0f, +1.0f, 1.0f, 1.0f,
		+1.0f, +1.0f, 1.0f, 1.0f,
		-1.0f, +1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f
	};
	quadVbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quadVertices), quadVertices);

	Mesh gridMesh;
	for(int i = 0; i <= 8; ++i)
	{
		float f = (i / 8.0) * 2.0f - 1.0f;
		int j = gridMesh.getPositionCount();
		gridMesh.addPosition(f * 3.0f, 0.0f, -3.0f);
		gridMesh.addPosition(f * 3.0f, 0.0f, +3.0f);
		gridMesh.addPosition(-3.0f, 0.0f, f * 3.0f);
		gridMesh.addPosition(+3.0f, 0.0f, f * 3.0f);
		gridMesh.addColor(Colors::White);
		gridMesh.addColor(Colors::White);
		gridMesh.addColor(Colors::White);
		gridMesh.addColor(Colors::White);
		gridMesh.addIndex(j + 0); gridMesh.addIndex(j + 1);
		gridMesh.addIndex(j + 2); gridMesh.addIndex(j + 3);
	}
	MeshBuffer gridBuffer(gridMesh);
	Model grid(gridBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	mat4 perspectiveMatrix = glm::perspective(45.0f, windowWidth / float(windowHeight), 0.05f, 50.0f);
	
	// The geometry to be refracted and reflected are stored in these
	// In addition to RGB values, the world-space height is stored in the alpha-channel
	// of the refraction texture.
	// Fresnel equations is used to blend between the two textures
	RenderTexture refractionRT(windowWidth, windowHeight);
	RenderTexture reflectionRT(windowWidth, windowHeight);

	renderer.setClearColor(0.55f, 0.45f, 0.45f, 1.0f);
	renderer.setClearDepth(1.0);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(context.isOpen())
	{
		timer.step();
		double time = timer.getElapsedTime();
		update(time, timer.getDelta());
		double renderStart = timer.getElapsedTime();

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);
		viewMatrix.rotateX(xAxisRotation);
		viewMatrix.rotateY(yAxisRotation);

		renderer.setCullState(CullStates::CullNone);
		renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);
		
		colorShader.begin();
		colorShader.setUniform("projection", perspectiveMatrix);
		cube.pushTransform();
		cube.translate(0.0f, 0.0f, 0.0f);
		cube.scale(0.5f);

		// Render the geometry to be refracted, store result in rt
		refractionRT.begin();
		renderer.clearColorAndDepth();
		colorShader.setUniform("view", viewMatrix.top());
		cube.draw(GL_TRIANGLES);
		grid.draw(GL_LINES);
		refractionRT.end();

		// Render the geometry to be reflected, store result in rt
		reflectionRT.begin();
		renderer.clearColorAndDepth();
		viewMatrix.push();
		viewMatrix.scale(1.0f, -1.0f, 1.0f); // Reflect about xz-plane
		colorShader.setUniform("view", viewMatrix.top());
		cube.draw(GL_TRIANGLES);
		viewMatrix.pop();
		reflectionRT.end();

		colorShader.end();
		cube.popTransform();

		// Render the water with the previous reflection/refraction texture
		waterShader.begin();
		waterShader.setUniform("time", time);
		glActiveTexture(GL_TEXTURE0 + 0);
		refractionRT.bindTexture();
		glActiveTexture(GL_TEXTURE0 + 1);
		reflectionRT.bindTexture();
		glActiveTexture(GL_TEXTURE0 + 2);
		waterNormals.bind();
		//waterShader.setUniform("view", viewMatrix.top());
		waterShader.setUniform("refraction_tex", 0);
		waterShader.setUniform("reflection_tex", 1);
		waterShader.setUniform("water_normals_tex", 2);
		//waterShader.setUniform("light0_pos", vec3(0.0f, 1.0f, 0.0f));
		//waterShader.setUniform("light0_col", vec3(1.0f, 0.8f, 0.5f));
		//waterShader.setUniform("ambient", vec3(67.0f/255.0f, 66.0f/255.0f, 63.0f/255.0f));
		quadVbo.bind();
		waterShader.setAttributefv("position", 2, 4, 0);
		waterShader.setAttributefv("texel", 2, 4, 2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		quadVbo.unbind();
		reflectionRT.unbindTexture();
		refractionRT.unbindTexture();
		waterNormals.unbind();
		waterShader.end();
		glActiveTexture(GL_TEXTURE0 + 0);

		// Render unmirrored scene
		//colorShader.begin();
		//renderer.clearColorAndDepth();
		//renderer.setCullState(CullStates::CullNone);
		//renderer.setBlendState(BlendStates::AlphaBlend);
		//renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);
		//colorShader.setUniform("projection", perspectiveMatrix);
		//colorShader.setUniform("view", viewMatrix.top());
		//cube.pushTransform();
		//cube.translate(0.0f, 0.4f, 0.0f);
		//cube.scale(0.5f);
		//cube.draw(GL_TRIANGLES);

		/*grid.pushTransform();
		grid.translate(0.0f, -0.5f, 0.0f);
		grid.draw(GL_LINES);
		grid.popTransform();*/

		// Draw mirrored scene to a rendertarget
		/*rt.begin();
		renderer.clearColorAndDepth();
		
		viewMatrix.push();
		viewMatrix.scale(1.0f, -1.0f, 1.0f);
		colorShader.setUniform("view", viewMatrix.top());
		cube.draw(GL_TRIANGLES);
		cube.popTransform();
		viewMatrix.pop();
		rt.end();*/

		// Enable stencil testing and mask out a section containing the water mesh
		//glEnable(GL_STENCIL_TEST);
		//glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		//glStencilMask(0xFF); // Write to stencil buffer
		//glDepthMask(GL_FALSE); // Don't write to depth buffer
		//glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

		//// Draw water mesh
		//water.pushTransform();
		//water.scale(3.0f);
		//water.draw(GL_TRIANGLES);
		//water.popTransform();
		//colorShader.end();

		//// Draw previous rendertarget as a quad masked into the water plane
		//glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
		//glStencilMask(0x00); // Don't write anything to stencil buffer
		//glDepthMask(GL_TRUE);

		//glDisable(GL_STENCIL_TEST);

		viewMatrix.pop();

		context.display();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			context.sleep(0.013 - renderTime);

		if(checkGLErrors(std::cerr))
		{
			std::cin.get();
			context.close();
		}
	}

	waterNormals.dispose();
	colorShader.dispose();
	waterShader.dispose();
	vao.dispose();
	context.dispose();
}

int main()
{
	try
	{
		run();
	}
	catch(std::exception &e)
	{
		std::cerr<<"An error occured: "<<e.what()<<std::endl;
		std::cin.get();
	}

	return 0;
}