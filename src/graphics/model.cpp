#include <graphics/model.h>

Model::Model(MeshBuffer &mesh) : 
	mb(&mesh), transform() 
{ }

Model::Model() : 
	mb(nullptr), transform() 
{ }

void Model::setMesh(MeshBuffer &mesh) 
{
	mb = &mesh; 
}

void Model::setTransform(const MatrixStack &t) 
{
	transform = t; 
}

void Model::rotateX(float rad) { transform.rotateX(rad); }
void Model::rotateY(float rad) { transform.rotateY(rad); }
void Model::rotateZ(float rad) { transform.rotateZ(rad); }
void Model::translate(const vec3 &v) { transform.translate(v); }
void Model::translate(float x, float y, float z) { transform.translate(x, y, z); }
void Model::scale(float sx, float sy, float sz) { transform.scale(sx, sy, sz); }
void Model::scale(float s) { transform.scale(s); }
void Model::multiply(const mat4 &m) { transform.multiply(m); }
void Model::pushTransform() { transform.push(); }
void Model::popTransform() { transform.pop(); }
const mat4 &Model::getTransform() const { return transform.top(); }

void Model::draw()
{
	if(!mb)
		throw std::exception("No associated MeshBuffer");

	ShaderProgram *sp = getActiveShader();
	if(!sp)
		throw std::exception("No active shader");

	sp->setUniform("model", transform.top());
	mb->draw();
}