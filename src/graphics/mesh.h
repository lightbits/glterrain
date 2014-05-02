#ifndef MESH_H
#define MESH_H
#include <vector>
#include <graphics/color.h>
#include <common/matrix.h>
#include <gl/opengl.h>

//enum ElementDrawMode
//{
//	TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
//	TRIANGLES = GL_TRIANGLES,
//	POINTS = GL_POINTS, 
//	LINE_STRIP = GL_LINE_STRIP, 
//	LINE_LOOP = GL_LINE_LOOP, 
//	LINES = GL_LINES, 
//	TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
//	TRIANGLE_FAN = GL_TRIANGLE_FAN,
//	TRIANGLES = GL_TRIANGLES
//};

class Mesh
{
public:
	Mesh();
	Mesh(GLenum mode);

	bool loadFromFile(const std::string &filename);

	void setDrawMode(GLenum mode);

	void addMesh(Mesh mesh);

	void addPosition(float x, float y, float z);
	void addPosition(const vec3 &p);
	void addPositions(const std::vector<vec3> &ps);
	void addPositions(const vec3 *ps, int count);

	void addNormal(float x, float y, float z);
	void addNormal(const vec3 &n);
	void addNormals(const std::vector<vec3> &ns);
	void addNormals(const vec3 *ns, int count);

	void addColor(float r, float g, float b, float a);
	void addColor(const vec4 &c);
	void addColor(const Color &c);
	void addColors(const std::vector<vec4> &cs);
	void addColors(const vec4 *cs, int count);

	void addTexel(float u, float v);
	void addTexel(const vec2 &t);
	void addTexels(const std::vector<vec2> &ts);
	void addTexels(const vec2 *ts, int count);

	void addIndex(unsigned int i);
	void addIndices(const std::vector<unsigned int> &is);
	void addIndices(const unsigned int *is, int count);
	void addLine(unsigned int i0, unsigned int i1);
	void addTriangle(unsigned int i0, unsigned int i1, unsigned int i2);

	vec3 &getPosition(int i);
	vec4 &getColor(int i);
	vec2 &getTexel(int i);
	vec3 &getNormal(int i);
	unsigned int &getIndex(int i);
	vec3 &getTangent(int i);
	vec3 &getBitangent(int i);

	int getPositionCount() const;
	int getColorCount() const;
	int getTexelCount() const;
	int getNormalCount() const;
	int getIndexCount() const;
	int getTangentsCount() const;

	int getPositionByteSize() const;
	int getColorByteSize() const;
	int getTexelByteSize() const;
	int getNormalByteSize() const;
	int getIndexByteSize() const;
	int getTangentsByteSize() const;
	int getByteSize() const;

	vec3 *getPositionPtr();
	vec4 *getColorPtr();
	vec2 *getTexelPtr();
	vec3 *getNormalPtr();
	vec3 *getTangentPtr();
	vec3 *getBitangentPtr();
	unsigned int *getIndexPtr();

	GLenum getDrawMode() const;

	void clearPositions();
	void clearNormals();
	void clearColors();
	void clearTexels();
	void clear();

	void calculateTangentVectors();
	void calculateNormalVectors();

	static Mesh genScreenSpaceTexQuad();
	static Mesh genPlane(const vec3 &up, const vec3 &right);
	static Mesh genPlane(float width, float height);
	static Mesh genUnitSphere(int tSamples, int sSamples);
	static Mesh genUnitCube(bool colors, bool normals);
	static Mesh genUnitColoredPlane(const Color &color);
	static Mesh genUnitCylinder(const Color &color, int levels);
	static Mesh genUnitGrid(const Color &color, int lines);

	//void draw() { getRenderer()->draw(*this, ...); }
private:
	std::vector<vec3> tangents;
	std::vector<vec3> bitangents;
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec4> colors;
	std::vector<vec2> texels;
	std::vector<unsigned int> indices;
	GLenum drawMode;
};

#endif