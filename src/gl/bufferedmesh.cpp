#include <gl/bufferedmesh.h>
#include <iostream>
#include <graphics/renderer.h>

MeshBuffer::MeshBuffer() : vaoOk(false), vbo(), ibo(), meshPtr(nullptr)
{

}

void MeshBuffer::dispose()
{
	vaoOk = false;
	vao.dispose();
	vbo.dispose();
	ibo.dispose();
	meshPtr = nullptr;
}

void MeshBuffer::create(Mesh &mesh)
{
	if(mesh.getIndexCount() == 0 || mesh.getPositionCount() == 0)
	{
		std::cerr<<"Error: Invalid mesh"<<std::endl;
		return;
	}

	dispose();
	meshPtr = &mesh;
	vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);

	int positionByteSize = mesh.getPositionCount() * sizeof(vec3);
	int normalByteSize = mesh.getNormalCount() * sizeof(vec3);
	int colorByteSize = mesh.getColorCount() * sizeof(vec4);
	int texelByteSize = mesh.getTexelCount() * sizeof(vec2);

	vbo.bind();
	vbo.bufferData(
		positionByteSize + 
		normalByteSize + 
		colorByteSize + 
		texelByteSize, NULL);

	int offset = 0;
	vbo.bufferSubData(offset, positionByteSize, mesh.getPositionPtr());
	offset += positionByteSize;

	if(mesh.getNormalCount() > 0)
	{
		vbo.bufferSubData(offset, normalByteSize, mesh.getNormalPtr());
		offset += normalByteSize;
	}
	if(mesh.getColorCount() > 0)
	{
		vbo.bufferSubData(offset, colorByteSize, mesh.getColorPtr());
		offset += colorByteSize;
	}
	if(mesh.getTexelCount() > 0)
	{
		vbo.bufferSubData(offset, texelByteSize, mesh.getTexelPtr());
	}

	vbo.unbind();

	ibo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
	ibo.bind();
	ibo.bufferData(mesh.getIndexCount() * sizeof(unsigned int), mesh.getIndexPtr());
	ibo.unbind();
}

void MeshBuffer::draw(GLenum drawMode)
{
	ShaderProgram *sp = getActiveShader();
	if(sp == nullptr)
	{
		std::cerr<<"Error: No active shader"<<std::endl;
		return;
	}
	else if(meshPtr->getIndexCount() == 0)
	{
		std::cerr<<"Error: No indices to be drawn"<<std::endl;
		return;
	}
		
	if(hasVao())
	{
		vao.bind();
		glDrawElements(drawMode, meshPtr->getIndexCount(), GL_UNSIGNED_INT, 0);
		vao.unbind();
	}
	else
	{
		vbo.bind();
		ibo.bind();
		int offset = 0;
		if(meshPtr->getPositionCount() > 0)
		{
			sp->setAttributefv("position", 3, 0, offset); 
			offset += meshPtr->getPositionCount() * 3;
		}
		if(meshPtr->getNormalCount() > 0)
		{
			sp->setAttributefv("normal", 3, 0, offset); 
			offset += meshPtr->getNormalCount() * 3;
		}
		if(meshPtr->getColorCount() > 0)
		{
			sp->setAttributefv("color", 4, 0, offset); 
			offset += meshPtr->getColorCount() * 4;
		}
		if(meshPtr->getTexelCount() > 0)
		{
			sp->setAttributefv("texel", 2, 0, offset); 
		}

		glDrawElements(drawMode, meshPtr->getIndexCount(), GL_UNSIGNED_INT, 0);
		vbo.unbind();
		ibo.unbind();
	}
}

void MeshBuffer::setupVao()
{
	/*vaoOk = true;
	vao.create();
	vao.bind();
	vbo.bind();
	ibo.bind();*/

	/*ShaderProgram *shader = getActiveShader();
	if(shader)
	{
		int offset = 0;
		if(meshPtr->getPositionCount() > 0)
		{
			shader->setAttributefv("position", 3, 0, offset);
			offset += 3;
		}
		if(meshPtr->getNormalCount() > 0)
		{
			shader->setAttributefv("normal", 3, 0, offset);
			offset += 3;
		}
		if(meshPtr->getColorCount() > 0)
		{
			shader->setAttributefv("color", 4, 0, offset);
			offset += 4;
		}
		if(meshPtr->getTexelCount() > 0)
		{
			shader->setAttributefv("texel", 2, 0, offset);
			offset += 2;
		}
	}
	else
	{
		std::cerr<<"Error: No active shader"<<std::endl;
	}*/

	/*vao.unbind();
	vbo.unbind();
	ibo.unbind();*/
}

void MeshBuffer::bind()
{
	if(hasVao())
	{
		vao.bind();
	}
	else
	{
		vbo.bind();
		ibo.bind();
	}
}

void MeshBuffer::unbind()
{
	if(hasVao())
	{
		vao.unbind();
	}
	else
	{
		vbo.unbind();
		ibo.unbind();
	}
}

BufferedMesh::BufferedMesh() : vbo(), ibo(), fmt(), indexCount(0)
{
	
}

void BufferedMesh::dispose()
{
	vbo.dispose();
	ibo.dispose();
}

void BufferedMesh::create(const TriMesh &mesh, const ProgramLayout &layout)
{
	dispose();

	vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	ibo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);

	fmt.clear();
	vbo.bind();
	vbo.bufferData(mesh.getByteSize(), NULL);
	GLintptr offset = 0;

	if(mesh.positions.size() > 0)
	{
		fmt.addAttrib(layout.getAttribLoc("position"), 3, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getPositionsByteSize(), &mesh.positions[0]);
		offset += mesh.getPositionsByteSize();
	}

	if(mesh.normals.size() > 0)
	{
		fmt.addAttrib(layout.getAttribLoc("normal"), 3, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getNormalsByteSize(), &mesh.normals[0]);
		offset += mesh.getNormalsByteSize();
	}

	if(mesh.colors.size() > 0)
	{
		fmt.addAttrib(layout.getAttribLoc("color"), 4, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getColorsByteSize(), &mesh.colors[0]);
		offset += mesh.getColorsByteSize();
	}

	if(mesh.texels.size() > 0)
	{
		fmt.addAttrib(layout.getAttribLoc("texel"), 2, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getTexCoordsByteSize(), &mesh.texels[0]);
	}

	indexCount = mesh.getIndexCount();
	ibo.bind();
	ibo.bufferData(indexCount * sizeof(GLushort), &mesh.indices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BufferedMesh::draw()
{
	if(indexCount == 0)
	{
		std::cerr<<"Error: Element array is empty"<<std::endl;
		return;
	}

	vbo.bind();
	ibo.bind();
	fmt.enable(); // We would be able to skip alot of these if we used an interlaced vertex format and batched the models
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
	fmt.disable();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}