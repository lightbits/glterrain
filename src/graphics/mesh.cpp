#include <graphics/mesh.h>
#include <common/typedefs.h>
#include <common/helpers.h>
#include <stdexcept>

Mesh::Mesh() : positions(), normals(), colors(), texels(), indices(), drawMode(GL_TRIANGLES) { }
Mesh::Mesh(GLenum mode) : drawMode(mode) { }

// http://paulbourke.net/dataformats/obj/
bool loadMeshObj(const std::string &filename, Mesh &mesh)
{
	mesh.clear();
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> texels;

	std::vector<std::string> lines;
	if(!readFile(filename, lines))
		return false;

	for(int i = 0; i < lines.size(); ++i)
	{
		std::stringstream ss(lines[i]);
		std::string prefix; ss>>prefix;
		if(prefix == "v") // Vertices
		{
			float x; ss>>x;
			float y; ss>>y;
			float z; ss>>z;
			positions.push_back(vec3(x, y, z));
		}
		else if(prefix == "vt") // Texture coordinates
		{
			float u; ss>>u;
			float v; ss>>v;
			texels.push_back(vec2(u, v));
		}
		else if(prefix == "vn") // Normals
		{
			float x; ss>>x;
			float y; ss>>y;
			float z; ss>>z;
			normals.push_back(vec3(x, y, z));
		}
		else if(prefix == "f") // Facets, assuming all other stuff loaded by now
		{
			// A single facet is in the format v/vt/vn, where
			// a texel or a normal may be omitted, as such:
			// v//vn, or v/vt/
			unsigned int i = mesh.getPositionCount();

			for(int n = 0; n < 3; ++n)
			{
				std::string indexStr; ss>>indexStr;
				std::vector<unsigned int> index = split<unsigned int>(indexStr, '/');
				// Remember that .obj indices start from 1
				mesh.addPosition(positions[index[0] - 1]);
				int j = 1;
				if(texels.size() > 0 && index.size() > j)
				{
					mesh.addTexel(texels[index[j] - 1]);
					++j;
				}
				if(normals.size() > 0 && index.size() > j)
				{
					mesh.addNormal(normals[index[j] - 1]);
				}
			}

			mesh.addTriangle(i + 0, i + 1, i + 2);
		}
	}

	return true;
}

bool Mesh::loadFromFile(const std::string &filename)
{
	std::string::size_type dot = filename.find_last_of('.');
	std::string ext = filename.substr(dot + 1);
	if(ext == "obj" && !loadMeshObj(filename, *this))
		return false;

	return true;
}

void Mesh::setDrawMode(GLenum mode) { drawMode = mode; }
GLenum Mesh::getDrawMode() const { return drawMode; }

void Mesh::addPosition(float x, float y, float z) { positions.push_back(vec3(x, y, z)); }
void Mesh::addPosition(const vec3 &p) { positions.push_back(p); }

void Mesh::addPositions(const std::vector<vec3> &ps) 
{
	// positions.reserve(positions.size() + ps.size());
	positions.insert(positions.end(), ps.begin(), ps.end());
}

void Mesh::addPositions(const vec3 *ps, int count) 
{
	// positions.reserve(positions.size() + count);
	// std::copy(ps.begin(), ps.end(), positions.end());
	positions.insert(positions.end(), ps, ps + count);
}

void Mesh::addNormal(float x, float y, float z) { normals.push_back(vec3(x, y, z)); }
void Mesh::addNormal(const vec3 &n) { normals.push_back(n); }

void Mesh::addNormals(const std::vector<vec3> &ns) 
{
	normals.insert(normals.end(), ns.begin(), ns.end());
}

void Mesh::addNormals(const vec3 *ns, int count)
{ 
	normals.insert(normals.end(), ns, ns + count);
}

void Mesh::addColor(float r, float g, float b, float a) { colors.push_back(vec4(r, g, b, a)); }
void Mesh::addColor(const vec4 &c) { colors.push_back(c); }
void Mesh::addColor(const Color &c) { colors.push_back(vec4(c.r, c.g, c.b, c.a)); }

void Mesh::addColors(const std::vector<vec4> &cs) 
{ 
	colors.insert(colors.end(), cs.begin(), cs.end());
}

void Mesh::addColors(const vec4 *cs, int count) 
{ 
	colors.insert(colors.end(), cs, cs + count);
}

void Mesh::addTexel(float u, float v) { texels.push_back(vec2(u, v)); }
void Mesh::addTexel(const vec2 &t) { texels.push_back(t); }

void Mesh::addTexels(const std::vector<vec2> &ts) 
{ 
	texels.insert(texels.end(), ts.begin(), ts.end());
}

void Mesh::addTexels(const vec2 *ts, int count) 
{ 
	texels.insert(texels.end(), ts, ts + count);
}

void Mesh::addIndex(unsigned int i) { indices.push_back(i); }

void Mesh::addIndices(const std::vector<unsigned int> &is) 
{ 
	indices.insert(indices.end(), is.begin(), is.end());
}

void Mesh::addIndices(const unsigned int *is, int count) 
{ 
	indices.insert(indices.end(), is, is + count);
}

void Mesh::addLine(unsigned int i0, unsigned int i1) 
{ 
	indices.push_back(i0);
	indices.push_back(i1);
}

void Mesh::addTriangle(unsigned int i0, unsigned int i1, unsigned int i2) 
{ 
	indices.push_back(i0);
	indices.push_back(i1);
	indices.push_back(i2);
}

vec3 &Mesh::getPosition(int i) { return positions[i]; }
vec4 &Mesh::getColor(int i) { return colors[i]; }
vec2 &Mesh::getTexel(int i) { return texels[i]; }
vec3 &Mesh::getNormal(int i) { return normals[i]; }
unsigned int &Mesh::getIndex(int i) { return indices[i]; }
vec3 &Mesh::getTangent(int i) { return tangents[i]; }
vec3 &Mesh::getBitangent(int i) { return bitangents[i]; }

int Mesh::getPositionCount() const { return positions.size(); }
int Mesh::getColorCount() const { return colors.size(); }
int Mesh::getTexelCount() const { return texels.size(); }
int Mesh::getNormalCount() const { return normals.size(); }
int Mesh::getIndexCount() const { return indices.size(); }
int Mesh::getTangentsCount() const { return tangents.size(); } /* assert(==bitangents.size()) */

int Mesh::getPositionByteSize() const { return positions.size() * sizeof(vec3); }
int Mesh::getColorByteSize() const  { return colors.size() * sizeof(vec4); }
int Mesh::getTexelByteSize() const  { return texels.size() * sizeof(vec2); }
int Mesh::getNormalByteSize() const  { return normals.size() * sizeof(vec3); }
int Mesh::getIndexByteSize() const  { return indices.size() * sizeof(unsigned int); }
int Mesh::getTangentsByteSize() const  { return tangents.size() * sizeof(vec3); }
int Mesh::getByteSize() const
{ 
	return 
		getPositionByteSize() + 
		getColorByteSize() +
		getTexelByteSize() + 
		getNormalByteSize() +
		getIndexByteSize() + 
		2 * getTangentsByteSize();
}

vec3 *Mesh::getPositionPtr() { return &positions[0]; }
vec4 *Mesh::getColorPtr() { return &colors[0]; }
vec2 *Mesh::getTexelPtr() { return &texels[0]; }
vec3 *Mesh::getNormalPtr() { return &normals[0]; }
vec3 *Mesh::getTangentPtr() { return &tangents[0]; }
vec3 *Mesh::getBitangentPtr() { return &bitangents[0]; }
unsigned int *Mesh::getIndexPtr() { return &indices[0]; }

void Mesh::clearPositions() { positions.clear(); }
void Mesh::clearNormals() { normals.clear(); }
void Mesh::clearColors() { colors.clear(); }
void Mesh::clearTexels() { texels.clear(); }

void Mesh::clear() 
{
	positions.clear();
	normals.clear();
	colors.clear();
	texels.clear();
	tangents.clear();
	bitangents.clear();
}

void Mesh::calculateTangentVectors()
{
	tangents.clear();
	bitangents.clear();
	throw std::runtime_error("Not yet implemented");
}

void Mesh::calculateNormalVectors()
{
	normals.clear();
	throw std::runtime_error("Not yet implemented");
}

Mesh Mesh::genUnitColoredCube()
{
	Mesh mesh;
	mesh.setDrawMode(GL_TRIANGLES);
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

Mesh Mesh::genUnitColoredPlane(const Color &color)
{
	Mesh mesh;
	mesh.setDrawMode(GL_TRIANGLES);
	mesh.addPosition(-0.5f, 0.0f, -0.5f);
	mesh.addPosition(+0.5f, 0.0f, -0.5f);
	mesh.addPosition(+0.5f, 0.0f, +0.5f);
	mesh.addPosition(-0.5f, 0.0f, +0.5f);
	mesh.addColor(color);
	mesh.addColor(color);
	mesh.addColor(color);
	mesh.addColor(color);
	mesh.addTriangle(0, 3, 2);
	mesh.addTriangle(2, 1, 0);
	return mesh;
}

Mesh Mesh::genUnitCylinder(const Color &color, int levels)
{
	Mesh mesh;
	mesh.setDrawMode(GL_TRIANGLES);
	float dt = TWO_PI / float(levels);
	for (int i = 0; i < levels; ++i)
	{
		unsigned int index = mesh.getPositionCount();
		float t = i * dt;
		float x0 = cos(t); float x1 = cos(t + dt);
		float y0 = sin(t); float y1 = sin(t + dt);
		mesh.addPosition(x0, y0, -1.0f);
		mesh.addPosition(x0, y0, +1.0f);
		mesh.addPosition(x1, y1, +1.0f);
		mesh.addPosition(x1, y1, -1.0f);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addTriangle(index, index + 1, index + 2);
		mesh.addTriangle(index + 2, index + 3, index);
	}
	return mesh;
}

Mesh Mesh::genUnitGrid(const Color &color, int lines)
{
	Mesh mesh;
	mesh.setDrawMode(GL_LINES);
	for (int i = 0; i <= lines; ++i)
	{
		unsigned int index = mesh.getPositionCount();
		float t = (i / float(lines)) * 2.0f - 1.0f;
		mesh.addPosition(t, 0.0f, -1.0f);
		mesh.addPosition(t, 0.0f, +1.0f);
		mesh.addPosition(-1.0f, 0.0f, t);
		mesh.addPosition(+1.0f, 0.0f, t);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addColor(color);
		mesh.addLine(index, index + 1);
		mesh.addLine(index + 2, index + 3);
	}
	return mesh;
}