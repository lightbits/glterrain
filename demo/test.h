struct StaticMesh
{
	Texture *texture;
	Program *program;
	GLuint vbo;
	GLuint vao;
};

struct DynamicMesh
{
	Texture *texture;
	Program *program;
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec4> colors;
	std::vector<vec2> texels;
};

struct StaticMeshInstance
{
	StaticMesh *mesh;
	mat4 modelTransform;
};


//
template <class T>
class VertexAttribArray
{
public:
	VertexAttribArray(GLenum dataType, int attribSize);
	void addData(T x);
	void addData(T x, T y);
	void addData(T x, T y, T z);
	void addData(T x, T y, T z, T w);

	void enable(GLint location, GLsizei stride, GLsizeiptr offset);
	void disable(GLint location);

	T *getDataPtr() const { return &data[0]; }
private:
	GLenum dataType;
	int size; // elements per attribute
	std::vector<T> data;
};

template <class T>
VertexAttribArray<T> &operator<<(VertexAttribArray<T> &lhs, T x) 
{
	lhs.addData(x);
	return lhs;
}

// Specialized class of vertex attrib arrays and index array
class TriMesh
{
public:

private:
	VertexAttribArray<GLfloat> positions;
	VertexAttribArray<GLfloat> normals;
	VertexAttribArray<GLfloat> colors;
	VertexAttribArray<GLfloat> texels;
	std::vector<GLushort> indices;
};