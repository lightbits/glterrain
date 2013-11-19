#ifndef MODEL_H
#define MODEL_H
#include <gl/shaderprogram.h>
#include <gl/bufferedmesh.h>
#include <graphics/renderer.h>
#include <common/matrixstack.h>
#include <common/vec.h>

/*
Convenience class for drawable mesh buffer and an associated model transformation
*/
class Model
{
public:
	Model(MeshBuffer &mesh) : mb(&mesh), transform() { }
	Model() : mb(nullptr), transform() { }
	void setMesh(MeshBuffer &mesh) { mb = &mesh; }
	void setTransform(const MatrixStack &t) { transform = t; }
	void rotateX(float rad) { transform.rotateX(rad); }
	void rotateY(float rad) { transform.rotateY(rad); }
	void rotateZ(float rad) { transform.rotateZ(rad); }
	void translate(const vec3 &v) { transform.translate(v); }
	void translate(float x, float y, float z) { transform.translate(x, y, z); }
	void scale(float sx, float sy, float sz) { transform.scale(sx, sy, sz); }
	void scale(float s) { transform.scale(s); }
	void pushTransform() { transform.push(); }
	void popTransform() { transform.pop(); }
	const mat4 &getTransform() const { return transform.top(); }
	void draw(GLenum drawMode)
	{
		if(!mb)
			throw std::exception("No associated MeshBuffer");

		ShaderProgram *sp = getActiveShader();
		if(!sp)
			throw std::exception("No active shader");		

		sp->setUniform("model", transform.top());
		mb->draw(drawMode);
	}
private:
	MeshBuffer *mb;
	MatrixStack transform;
};

#endif