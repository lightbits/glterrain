#ifndef MODEL_H
#define MODEL_H
#include <gl/shaderprogram.h>
#include <gl/meshbuffer.h>
#include <common/matrixstack.h>
#include <common/matrix.h>
#include <common/transform.h>

/*
Convenience class for drawable mesh buffer and an associated model transformation
*/
struct Model
{
	MeshBuffer *mesh;
	//Material *material;
	MatrixStack transform;

	Model();
	Model(MeshBuffer &mesh, const mat4 &transform = mat4(1.0f));
	void draw();
};

//class Model
//{
//public:
//	Model(MeshBuffer &mesh);
//	Model();
//	void setMesh(MeshBuffer &mesh);
//	void setTransform(const MatrixStack &t);
//	void rotateX(float rad);
//	void rotateY(float rad);
//	void rotateZ(float rad);
//	void translate(const vec3 &v);
//	void translate(float x, float y, float z);
//	void scale(float sx, float sy, float sz);
//	void scale(float s);
//	void multiply(const mat4 &m);
//	void multiply(const quat &q);
//	void pushTransform();
//	void popTransform();
//	const mat4 &getTransform() const;
//	void draw();
//private:
//	MeshBuffer *mb;
//	MatrixStack transform;
//};

#endif