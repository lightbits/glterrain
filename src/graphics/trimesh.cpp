#include <graphics/trimesh.h>
#include <common/helpers.h>
#include <fstream>
#include <iostream>
#include <string>

bool TriMesh::loadFromFile(const char *filename, bool flipWindingOrder)
{
	std::ifstream in(filename);
	if(!in.is_open())
		return false;

	clear();

	std::vector<vec3> vertices;
	std::vector<vec2> texCoords;
	std::vector<vec3> normals;

	bool hasTexCoords = false;
	bool hasNormals = false;

	while(in.good())
	{
		std::string prefix;
		in>>prefix;
		if(prefix == "v") // Position (vertex)
		{
			float x, y, z;
			in>>x; in>>y; in>>z;
			vertices.push_back(vec3(x, y, z));
		}
		else if(prefix == "vt") // Texture coordinate
		{
			float u, v;
			in>>u; in>>v;
			texCoords.push_back(vec2(u, v));
			hasTexCoords = true;
		}
		else if(prefix == "vn") // Normal
		{
			float x, y, z;
			in>>x; in>>y; in>>z;
			normals.push_back(vec3(x, y, z));
			hasNormals = true;
		}
		else if(prefix == "f") // Face/triangle
		{
			/*
			Each line starting with 'f' consists of three triplets of indices, position/uv/normal,
			which together make up one face (triangle).
			Because we can't use seperate index arrays for each attribute, we need to append the vertex
			data to the mesh for each index

			Note that indices start from 1, and not 0, in .obj files.
			*/
			for(int i = 0; i < 3; ++i)
			{
				std::string vertex;
				in>>vertex;
				std::vector<int> indices = split<int>(vertex, '/');
				if(indices.size() != 1 + (int)hasTexCoords + (int)hasNormals)
					return false;

				addPosition(vertices[indices[0] - 1]);

				if(hasTexCoords)
					addTexel(texCoords[indices[1] - 1]);

				if(hasNormals)
					addNormal(normals[indices[2] - 1]);
			}

			// Add the indices for the face
			unsigned short i = this->getLastVertexIndex() - 2;
			if(flipWindingOrder)
				addTriangle(i + 2, i + 1, i + 0);
			else
				addTriangle(i + 0, i + 1, i + 2);
		}
	}

	unsigned int positionsbytes = getPositionCount() * sizeof(vec3);
	unsigned int texcoordsbytes = getTexCoordCount() * sizeof(vec2);
	unsigned int normalsbytes = getNormalCount() * sizeof(vec3);

	std::cout<<"loaded model ("<<filename<<")"<<std::endl;
	std::cout<<"\t"<<getPositionCount()<<" vertices ("<<positionsbytes / 1024<<"kB)"<<std::endl;
	std::cout<<"\t"<<getTexCoordCount()<<" UV coordinates ("<<texcoordsbytes / 1024<<"kb)"<<std::endl;
	std::cout<<"\t"<<getNormalCount()<<" normals ("<<normalsbytes / 1024<<"kB)"<<std::endl;
	std::cout<<"\t"<<getIndexCount() / 3<<" faces in total ("<<(positionsbytes + texcoordsbytes + normalsbytes) / 1024<<"kB)"<<std::endl;

	return true;
}

TriMesh TriMesh::genUnitColoredCube()
{
	TriMesh mesh;
	const float halfSize = 0.5f;

	// All faces are oriented counter-clockwise outwards
	
	// Front face
	mesh.addPosition(-halfSize, -halfSize, halfSize); mesh.addColor(1.0f, 0.4f, 0.4f, 1.0f); 
	mesh.addPosition( halfSize, -halfSize, halfSize); mesh.addColor(1.0f, 0.4f, 0.4f, 1.0f); 
	mesh.addPosition( halfSize,  halfSize, halfSize); mesh.addColor(1.0f, 0.4f, 0.4f, 1.0f); 
	mesh.addPosition(-halfSize,  halfSize, halfSize); mesh.addColor(1.0f, 0.4f, 0.4f, 1.0f);
	mesh.addTriangle(0, 1, 2);
	mesh.addTriangle(2, 3, 0);

	// Back face
	mesh.addPosition( halfSize, -halfSize, -halfSize); mesh.addColor(0.4f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition(-halfSize, -halfSize, -halfSize); mesh.addColor(0.4f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition(-halfSize,  halfSize, -halfSize); mesh.addColor(0.4f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition( halfSize,  halfSize, -halfSize); mesh.addColor(0.4f, 1.0f, 0.4f, 1.0f);
	mesh.addTriangle(4, 5, 6);
	mesh.addTriangle(6, 7, 4);

	// Left face
	mesh.addPosition(-halfSize, -halfSize, -halfSize); mesh.addColor(1.0f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize, -halfSize,  halfSize); mesh.addColor(1.0f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize,  halfSize,  halfSize); mesh.addColor(1.0f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize,  halfSize, -halfSize); mesh.addColor(1.0f, 0.4f, 1.0f, 1.0f);
	mesh.addTriangle(8, 9, 10);
	mesh.addTriangle(10, 11, 8);

	// Right face
	mesh.addPosition( halfSize, -halfSize,  halfSize); mesh.addColor(1.0f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition( halfSize, -halfSize, -halfSize); mesh.addColor(1.0f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition( halfSize,  halfSize, -halfSize); mesh.addColor(1.0f, 1.0f, 0.4f, 1.0f);
	mesh.addPosition( halfSize,  halfSize,  halfSize); mesh.addColor(1.0f, 1.0f, 0.4f, 1.0f);
	mesh.addTriangle(12, 13, 14);
	mesh.addTriangle(14, 15, 12);

	// Top face
	mesh.addPosition(-halfSize,  halfSize,  halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition( halfSize,  halfSize,  halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition( halfSize,  halfSize, -halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize,  halfSize, -halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addTriangle(16, 17, 18);
	mesh.addTriangle(18, 19, 16);

	// Bottom face
	mesh.addPosition( halfSize, -halfSize,  halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize, -halfSize,  halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition(-halfSize, -halfSize, -halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addPosition( halfSize, -halfSize, -halfSize); mesh.addColor(0.4f, 0.4f, 1.0f, 1.0f);
	mesh.addTriangle(20, 21, 22);
	mesh.addTriangle(22, 23, 20);

	return mesh;
}

vec3 bezier(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3, float t)
{
	return vec3(bezier(v0.x, v1.x, v2.x, v3.x, t), 
		bezier(v0.y, v1.y, v2.y, v3.y, t),
		bezier(v0.z, v1.z, v2.z, v3.z, t)
		);
}

// Performs a 2d bezier interpolation on the points.
// The first index in the point array is interpolated by v, and
// the second index by h.
vec3 bezier(const vec3 points[4][4], float h, float v)
{
	return bezier(
		bezier(points[0][0], points[1][0], points[2][0], points[3][0], v),
		bezier(points[0][1], points[1][1], points[2][1], points[3][1], v),
		bezier(points[0][2], points[1][2], points[2][2], points[3][2], v),
		bezier(points[0][3], points[1][3], points[2][3], points[3][3], v),
		h);
}

TriMesh TriMesh::genBezierPatch(const vec3 points[4][4], int vSamples, int hSamples)
{
	TriMesh mesh;

	float vStep = 1.0f / (float)vSamples;
	float hStep = 1.0f / (float)hSamples;
	// Remember, floating points are not that accurate, so instead of iterating up to and including 1.0f - *Step,
	// we just iterate while * < 1.0f
	for(float v = 0.0f; v < 1.0f; v += vStep)
	{
		for(float h = 0.0; h < 1.0f; h += hStep)
		{
			vec3 topLeft = bezier(points, h, v);
			vec3 topRight = bezier(points, h + hStep, v);
			vec3 bottomLeft = bezier(points, h, v + vStep);
			vec3 bottomRight = bezier(points, h + hStep, v + vStep);

			mesh.addPosition(topLeft);		mesh.addColor(v, h, 1.0f, 1.0f); // i - 3
			mesh.addPosition(topRight);		mesh.addColor(v, h + hStep, 1.0f, 1.0f); // i - 2
			mesh.addPosition(bottomLeft);	mesh.addColor(v + vStep, h, 1.0f, 1.0f); // i - 1
			mesh.addPosition(bottomRight);	mesh.addColor(v + vStep, h + hStep, 1.0f, 1.0f); // i

			unsigned int i = mesh.getLastVertexIndex();
			mesh.addTriangle(i - 3, i - 1, i - 2);
			mesh.addTriangle(i - 1, i, i - 2);
		}
	}

	return mesh;
}