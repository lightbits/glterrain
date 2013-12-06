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
#include <app/glcontext.h>
#include "noise.h"

void initTerrainMesh(Mesh &mesh, 
					 float frequency, 
					 int samples, // Number of interpolation values to take between start and end noise points
					 float scale, // World-space width and depth
					 float height) // World-space height
{
	mesh.clear();
	float ds = frequency / float(samples);

	std::vector<float> heights(samples * samples);
	for(int i = 0; i < heights.size(); ++i)
	{
		int xi = i % samples;
		int yi = i / samples;
		//heights[i] = fBm(xi * ds, yi * ds);
		float xf = (xi / float(samples)) * 2.0f - 1.0f;
		float yf = (yi / float(samples)) * 2.0f - 1.0f;
		float rsqrd = xf * xf + yf * yf;
		float h0 = fBm(xi * ds, yi * ds);
		float h1 = 1.5f * exp(-rsqrd * 1.3f);
		heights[i] = h0 * h1 * height;
	}

	/*std::vector<vec3> normals(samples * samples);
	for(int y = 1; y < samples - 1; ++y)
	{
		for(int x = 1; x < samples - 1; ++x)
		{
			float hCenter = heights[y * samples + x];
			float hTop = heights[(y - 1) * samples + x];
			float hBottom = heights[(y + 1) * samples + x];
			float hLeft = heights[y * samples + x - 1];
			float hRight = heights[y * samples + x + 1];

			float dx = scale / float(samples);
			float dz = scale / float(samples);
			vec3 v0 = vec3(dx, hRight - hCenter, 0.0f);
			vec3 v1 = vec3(0.0f, hTop - hCenter, -dz);
			vec3 v2 = vec3(-dx, hLeft - hCenter, 0.0f);
			vec3 v3 = vec3(0.0f, hBottom - hCenter, dz);

			vec3 n0 = glm::normalize(glm::cross(v0, v1));
			vec3 n1 = glm::normalize(glm::cross(v1, v2));
			vec3 n2 = glm::normalize(glm::cross(v2, v3));
			vec3 n3 = glm::normalize(glm::cross(v3, v0));
			normals[y * samples + x] = glm::normalize(n0 + n1 + n2 + n3);
		}
	}*/

	for(int y = 0; y < samples - 1; ++y)
	{
		for(int x = 0; x < samples - 1; ++x)
		{
			int i00 = y * samples + x;
			int i10 = y * samples + x + 1;
			int i01 = (y + 1) * samples + x;
			int i11 = (y + 1) * samples + x + 1;
			float h00 = heights[i00];
			float h10 = heights[i10];
			float h01 = heights[i01];
			float h11 = heights[i11];

			// Model-scale coordinates
			float xf0 = scale * ((x + 0) / float(samples - 1) - 0.5f);
			float xf1 = scale * ((x + 1) / float(samples - 1) - 0.5f);
			float zf0 = scale * ((y + 0) / float(samples - 1) - 0.5f);
			float zf1 = scale * ((y + 1) / float(samples - 1) - 0.5f);

			vec3 v00 = vec3(xf0, h00, zf0); // top-left
			vec3 v10 = vec3(xf1, h10, zf0); // top-right
			vec3 v01 = vec3(xf0, h01, zf1); // bottom-left
			vec3 v11 = vec3(xf1, h11, zf1); // bottom-right

			const vec3 vertices[] = { v00, v10, v11, v11, v01, v00 };
			int i = mesh.getPositionCount();
			mesh.addPositions(vertices, 6);

			vec3 n0 = glm::normalize(glm::cross(v00 - v10, v11 - v10));
			vec3 n1 = glm::normalize(glm::cross(v11 - v01, v00 - v01));
			mesh.addNormal(n0);
			mesh.addNormal(n0);
			mesh.addNormal(n0);

			mesh.addNormal(n1);
			mesh.addNormal(n1);
			mesh.addNormal(n1);
			/*mesh.addNormal(normals[i00]);
			mesh.addNormal(normals[i10]);
			mesh.addNormal(normals[i11]);

			mesh.addNormal(normals[i11]);
			mesh.addNormal(normals[i01]);
			mesh.addNormal(normals[i00]);*/

			mesh.addTriangle(i + 0, i + 1, i + 2);
			mesh.addTriangle(i + 3, i + 4, i + 5);
		}
	}
}

int main()
{
	GLContext context;
	if(!context.create("Terrain", VideoMode(640, 480, 24, 8, 8, false)))
		shutdown("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram 
		terrainShader,
		waterShader,
		simpleShader;

	if(!terrainShader.loadFromFile("data/shaders/terrain.vert", "data/shaders/terrain.frag") ||
	   !waterShader.loadFromFile("data/shaders/water.vert", "data/shaders/water.frag") ||
	   !simpleShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag"))
		shutdown("Failed to load shaders");

	if(!terrainShader.linkAndCheckStatus() ||
	   !waterShader.linkAndCheckStatus() ||
	   !simpleShader.linkAndCheckStatus())
		shutdown("Failed to link shaders");

	Font font0;
	if(!font0.loadFromFile("data/fonts/proggytinyttsz_8x12.png"))
		shutdown("Failed to load fonts");

	SpriteBatch spriteBatch;
	spriteBatch.setFont(font0);

	Mesh terrainMesh;
	initTerrainMesh(terrainMesh, 1.4f, 128, 5.0f, 2.0f);
	MeshBuffer terrainBuffer(terrainMesh);
	Model terrain(terrainBuffer);

	Mesh waterMesh = Mesh::genUnitColoredPlane(Color(0.22f, 0.41f, 0.61, 0.4f));
	MeshBuffer waterBuffer(waterMesh);
	Model water(waterBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	renderer.setClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	renderer.setClearDepth(1.0);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	float yAxisRotation = 0.0f;
	float xAxisRotation = -0.78f;

	while(context.isOpen())
	{
		timer.step();
		float time = timer.getElapsedTime();

		if(glfwGetKey('R'))
		{
			if(!terrainShader.loadFromFile("data/shaders/terrain.vert", "data/shaders/terrain.frag") ||
			   !waterShader.loadFromFile("data/shaders/water.vert", "data/shaders/water.frag") ||
			   !simpleShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag"))
				shutdown("Failed to load shaders");

			if(!terrainShader.linkAndCheckStatus() ||
			   !waterShader.linkAndCheckStatus() ||
			   !simpleShader.linkAndCheckStatus())
				shutdown("Failed to link shaders");
		}

		if(glfwGetKey(GLFW_KEY_LEFT))
			yAxisRotation -= 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_RIGHT))
			yAxisRotation += 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_UP))
			xAxisRotation += 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_DOWN))
			xAxisRotation -= 0.78f * timer.getDelta();

		double renderStart = timer.getElapsedTime();
		renderer.clearColorAndDepth();

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -4.0f);
		viewMatrix.rotateX(xAxisRotation);
		viewMatrix.rotateY(yAxisRotation);

		renderer.setCullState(CullStates::CullNone);
		renderer.setBlendState(BlendStates::AlphaBlend);
		renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);

		terrainShader.begin();
		terrainShader.setUniform("projection", perspectiveMatrix);
		terrainShader.setUniform("view", viewMatrix.top());
		terrainShader.setUniform("light0_pos", vec3(0.0f, 2.0f, 2.0f));
		terrainShader.setUniform("light0_col", vec3(1.0f, 0.8f, 0.5f));
		terrainShader.setUniform("ambient", vec3(67.0f/255.0f, 66.0f/255.0f, 63.0f/255.0f));		
		terrain.draw(GL_TRIANGLES);
		terrainShader.end();

		renderer.setBlendState(BlendStates::AlphaBlend);
		simpleShader.begin();
		simpleShader.setUniform("projection", perspectiveMatrix);
		simpleShader.setUniform("view", viewMatrix.top());
		water.pushTransform();
		water.translate(0.0f, -0.04f, 0.0f);
		water.scale(5.0f);
		water.draw(GL_TRIANGLES);
		water.popTransform();
		simpleShader.end();

		viewMatrix.pop();

		Text debugInfo;
		debugInfo<<"render: "<<int(renderTime * 1000.0)<<"ms";

		spriteBatch.begin();
		spriteBatch.drawString(debugInfo.getString(), vec2(5.0f, 5.0f), Color(0.44f, 0.55f, 0.89f, 1.0f));
		spriteBatch.end();

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

	terrainShader.dispose();
	vao.dispose();
	font0.dispose();
	return 0;
}

/*if(glfwGetKey('N') && terrainMesh.getNormalCount() > 0)
		{
			Mesh normals;
			for(int i = 0; i < terrainMesh.getIndexCount(); ++i)
			{
				unsigned int a0 = terrainMesh.getIndex(i);
				vec3 p = terrainMesh.getPosition(a0);
				vec3 n = terrainMesh.getNormal(a0);
				unsigned int j = normals.getPositionCount();
				normals.addPosition(p);
				normals.addPosition(p + n);
				normals.addColor(Color(255, 20, 20));
				normals.addColor(Color(255, 20, 20));
				normals.addLine(j, j + 1);
			}
			MeshBuffer mb(normals);
			simpleShader.begin();
			simpleShader.setUniform("projection", perspectiveMatrix);
			simpleShader.setUniform("view", viewMatrix.top());
			simpleShader.setUniform("model", mat4(1.0f));
			mb.draw(GL_LINES);
			mb.dispose();
			simpleShader.end();
		}*/

		/*if(glfwGetKey('N'))
		{
			Mesh normals;
			for(int i = 0; i < terrainMesh.getIndexCount() - 2; i += 3)
			{
				unsigned int a0 = terrainMesh.getIndex(i);
				unsigned int a1 = terrainMesh.getIndex(i + 1);
				unsigned int a2 = terrainMesh.getIndex(i + 2);

				vec3 v0 = terrainMesh.getPosition(a0);
				vec3 v1 = terrainMesh.getPosition(a1);
				vec3 v2 = terrainMesh.getPosition(a2);

				vec3 n = glm::normalize(glm::cross(v0 - v1, v2 - v1));

				unsigned int j = normals.getPositionCount();
				vec3 center = v1 + 0.5f * (v0 - v1) + 0.5f * (v2 - v1);
				Color color = i % 2 == 0 ? Color(255, 20, 20) : Color(20, 20, 255);
				normals.addPosition(v0); normals.addPosition(v0 + n);
				normals.addPosition(v1); normals.addPosition(v1 + n);
				normals.addPosition(v2); normals.addPosition(v2 + n);
				normals.addColor(color); normals.addColor(color);
				normals.addColor(color); normals.addColor(color);
				normals.addColor(color); normals.addColor(color);
				normals.addIndex(j + 0); normals.addIndex(j + 1);
				normals.addIndex(j + 2); normals.addIndex(j + 3);
				normals.addIndex(j + 4); normals.addIndex(j + 5);
			}
			MeshBuffer mb(normals);
			Model m(mb);
			simpleShader.begin();
			simpleShader.setUniform("projection", perspectiveMatrix);
			simpleShader.setUniform("view", viewMatrix.top());
			m.draw(GL_LINES);
			simpleShader.end();
			mb.dispose();
		}*/