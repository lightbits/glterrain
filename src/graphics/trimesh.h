#ifndef SLGL_GRAPHICS_TRIMESH_H
#define SLGL_GRAPHICS_TRIMESH_H
#include <vector>
#include <graphics/color.h>
#include <common/vec.h>

namespace graphics
{

class TriMesh
{
public:
	// Generates a cube mesh of size 1
	static TriMesh genUnitColoredCube();

	// Generates a smooth surface from the sample points
	static TriMesh genBezierPatch(const vec3 points[4][4], int vSamples, int hSamples);
public:
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec4> colors;
	std::vector<vec2> texCoords;
	std::vector<unsigned short> indices;

	// Loads data from a .obj model file
	bool loadFromFile(const char *filename, bool flipWindingOrder);

	void clear()
	{ 
		positions.clear(); normals.clear();
		colors.clear(); texCoords.clear();
		indices.clear();
	}

	void addPosition(const vec3 &p) { positions.push_back(p); }
	void addPosition(float x, float y, float z) { addPosition(vec3(x, y, z)); }

	void addNormal(const vec3 &n) { normals.push_back(n); }
	void addNormal(float x, float y, float z) { addNormal(vec3(x, y, z)); }

	void addColor(const Color &color) { colors.push_back(vec4(color.r, color.g, color.b, color.a)); }
	void addColor(float r, float g, float b, float a) { colors.push_back(vec4(r, g, b, a)); }

	void addTexCoord(const vec2 &uv) { texCoords.push_back(uv); }
	void addTexCoord(float u, float v) { addTexCoord(vec2(u, v)); }

	void addTriangle(unsigned short i0, unsigned short i1, unsigned short i2)
	{
		indices.push_back(i0); 
		indices.push_back(i1); 
		indices.push_back(i2); 
	}

	unsigned short getLastVertexIndex()	const { return positions.size() - 1; }
	unsigned int getPositionCount()		const { return positions.size(); }
	unsigned int getNormalCount()		const { return normals.size(); }
	unsigned int getColorCount()		const { return colors.size(); }
	unsigned int getTexCoordCount()		const { return texCoords.size(); }
	unsigned int getIndexCount()		const { return indices.size(); }

	unsigned int getPositionsByteSize() const { return positions.size() * sizeof(vec3); }
	unsigned int getNormalsByteSize()	const { return normals.size() * sizeof(vec3); }
	unsigned int getColorsByteSize()	const { return colors.size() * sizeof(vec4); }
	unsigned int getTexCoordsByteSize() const { return texCoords.size() * sizeof(vec2); }

	unsigned int getByteSize() const 
	{
		return positions.size() * sizeof(vec3) + 
			normals.size() * sizeof(vec3) + 
			colors.size() * sizeof(vec4) + 
			texCoords.size() * sizeof(vec2); 
	}
};

}

#endif