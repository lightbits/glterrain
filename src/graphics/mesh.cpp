#include <graphics/mesh.h>

Mesh::Mesh() : positions(), normals(), colors(), texels(), indices(), drawMode(GL_TRIANGLES) { }
Mesh::Mesh(GLenum mode) { }

void Mesh::setDrawMode(GLenum mode) { drawMode = mode; }
GLenum Mesh::getDrawMode() const { return drawMode; }

void Mesh::addPosition(float x, float y, float z) { positions.push_back(vec3(x, y, z)); }
void Mesh::addPosition(const vec3 &p) { positions.push_back(p); }
void Mesh::addPositions(const std::vector<vec3> &ps) {
	// positions.reserve(positions.size() + ps.size());
	positions.insert(positions.end(), ps.begin(), ps.end());
}
void Mesh::addPositions(const vec3 *ps, int count) {
	// positions.reserve(positions.size() + count);
	// std::copy(ps.begin(), ps.end(), positions.end());
	positions.insert(positions.end(), ps, ps + count);
}

void Mesh::addNormal(float x, float y, float z) { normals.push_back(vec3(x, y, z)); }
void Mesh::addNormals(const std::vector<vec3> &ns) {
	normals.insert(normals.end(), ns.begin(), ns.end());
}
void Mesh::addNormals(const vec3 *ns, int count) { 
	normals.insert(normals.end(), ns, ns + count);
}

void Mesh::addColor(float r, float g, float b, float a) { colors.push_back(vec4(r, g, b, a)); }
void Mesh::addColor(const vec4 &c) { colors.push_back(c); }
void Mesh::addColors(const std::vector<vec4> &cs) { 
	colors.insert(colors.end(), cs.begin(), cs.end());
}
void Mesh::addColors(const vec4 *cs, int count) { 
	colors.insert(colors.end(), cs, cs + count);
}

void Mesh::addTexel(float u, float v) { texels.push_back(vec2(u, v)); }
void Mesh::addTexel(const vec2 &t) { texels.push_back(t); }
void Mesh::addTexels(const std::vector<vec2> &ts) { 
	texels.insert(texels.end(), ts.begin(), ts.end());
}
void Mesh::addTexels(const vec2 *ts, int count) { 
	texels.insert(texels.end(), ts, ts + count);
}

void Mesh::addIndex(unsigned int i) { indices.push_back(i); }
void Mesh::addIndices(const std::vector<unsigned int> &is) { 
	indices.insert(indices.end(), is.begin(), is.end());
}
void Mesh::addIndices(const unsigned int *is, int count) { 
	indices.insert(indices.end(), is, is + count);
}
void Mesh::addTriangle(unsigned int i0, unsigned int i1, unsigned int i2) { 
	indices.push_back(i0);
	indices.push_back(i1);
	indices.push_back(i2);
}

int Mesh::getPositionCount() const { return positions.size(); }
int Mesh::getColorCount() const { return colors.size(); }
int Mesh::getTexelCount() const { return texels.size(); }
int Mesh::getNormalCount() const { return normals.size(); }
int Mesh::getIndexCount() const { return indices.size(); }

vec3 *Mesh::getPositionPtr() { return &positions[0]; }
vec4 *Mesh::getColorPtr() { return &colors[0]; }
vec2 *Mesh::getTexelPtr() { return &texels[0]; }
vec3 *Mesh::getNormalPtr() { return &normals[0]; }
unsigned int *Mesh::getIndexPtr() { return &indices[0]; }

void Mesh::clearPositions() { positions.clear(); }
void Mesh::clearNormals() { normals.clear(); }
void Mesh::clearColors() { colors.clear(); }
void Mesh::clearTexels() { texels.clear(); }

void Mesh::clear() {
	positions.clear();
	normals.clear();
	colors.clear();
	texels.clear();
}

Mesh Mesh::getUnitColoredCube()
{
	Mesh mesh;
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
