#include <gl/bufferedmesh.h>
#include <iostream>
#include <graphics/renderer.h>

MeshBuffer::MeshBuffer() : vaoOk(false), interleavedVertexData(false), vbo(), ibo(), stride(0), meshPtr(nullptr)
{

}

MeshBuffer::MeshBuffer(Mesh &mesh) : vaoOk(false), 
interleavedVertexData(false), vbo(), ibo(), stride(0), meshPtr(nullptr)
{
	create(mesh);
}

void MeshBuffer::dispose()
{
	stride = 0;
	interleavedVertexData = false;
	vaoOk = false;
	vao.dispose();
	vbo.dispose();
	ibo.dispose();
	meshPtr = nullptr;
}

void MeshBuffer::create(Mesh &mesh)
{
	if(mesh.getIndexCount() == 0 || mesh.getPositionCount() == 0)
		throw std::exception("Cannot create mesh buffer from empty mesh");

	dispose();
	meshPtr = &mesh;

	//interleavedVertexData = interleaved;

	// Assume that if data is interleaved, the mesh will not be updated often,
	// while if it is not interleaved, it is updated regularly.
	GLenum usage = interleavedVertexData ? GL_STATIC_DRAW : GL_STREAM_DRAW;

	vbo.create(GL_ARRAY_BUFFER, usage);
	vbo.bind();
	vbo.bufferData(mesh.getByteSize(), NULL);
	if(interleavedVertexData) 
		bufferMeshInterleaved(mesh);
	else 
		bufferMeshBlock(mesh);
	vbo.unbind();

	ibo.create(GL_ELEMENT_ARRAY_BUFFER, usage);
	ibo.bind();
	ibo.bufferData(mesh.getIndexCount() * sizeof(unsigned int), mesh.getIndexPtr());
	ibo.unbind();
}

void MeshBuffer::bufferMeshBlock(Mesh &mesh)
{
	int positionByteSize = mesh.getPositionByteSize();
	int normalByteSize = mesh.getNormalByteSize();
	int colorByteSize = mesh.getColorByteSize();
	int texelByteSize = mesh.getTexelByteSize();
	int tangentsByteSize = mesh.getTangentsByteSize();

	int offset = 0;
	vbo.bufferSubData(offset, positionByteSize, mesh.getPositionPtr());
	offset += positionByteSize;

	if(mesh.getNormalCount())
	{
		vbo.bufferSubData(offset, normalByteSize, mesh.getNormalPtr());
		offset += normalByteSize;
	}
	if(mesh.getColorCount())
	{
		vbo.bufferSubData(offset, colorByteSize, mesh.getColorPtr());
		offset += colorByteSize;
	}
	if(mesh.getTexelCount())
	{
		vbo.bufferSubData(offset, texelByteSize, mesh.getTexelPtr());
		offset += texelByteSize;
	}
	if(mesh.getTangentsCount())
	{
		vbo.bufferSubData(offset, tangentsByteSize, mesh.getTangentPtr());
		vbo.bufferSubData(offset, tangentsByteSize, mesh.getBitangentPtr());
		offset += 2 * tangentsByteSize;
	}
}

void MeshBuffer::bufferMeshInterleaved(Mesh &mesh)
{
	int positionByteSize = mesh.getPositionByteSize();
	int normalByteSize = mesh.getNormalByteSize();
	int colorByteSize = mesh.getColorByteSize();
	int texelByteSize = mesh.getTexelByteSize();
	int tangentsByteSize = mesh.getTangentsByteSize();

	stride = 0;
	if(positionByteSize) stride += 3;
	if(normalByteSize) stride += 3;
	if(colorByteSize) stride += 4;
	if(texelByteSize) stride += 2;
	if(tangentsByteSize) stride += 2 * 3;

	typedef struct VertexData
	{
		vec3 position;
		vec3 normal;
		vec4 color;
		vec2 texel;
		vec3 tangent;
		vec3 bitangent;
	} vertexData;

	int vertexCount = mesh.getPositionCount();
	vertexData *data = new vertexData[vertexCount];
	for(int i = 0; i < vertexCount; ++i)
	{
		data[i].position = *(mesh.getPositionPtr() + i);
		if(normalByteSize)
			data[i].normal = *(mesh.getNormalPtr() + i);
		if(colorByteSize)
			data[i].color = *(mesh.getColorPtr() + i);
		if(texelByteSize)
			data[i].texel = *(mesh.getTexelPtr() + i);
		if(tangentsByteSize)
		{
			data[i].tangent = *(mesh.getTangentPtr() + i);
			data[i].bitangent = *(mesh.getBitangentPtr() + i);
		}
	}

	vbo.bufferSubData(0, vertexCount * sizeof(vertexData), data);
	delete[] data;
}

void MeshBuffer::update(Mesh &mesh)	
{
	update(mesh, 0, mesh.getIndexCount());
}

void MeshBuffer::update(Mesh &mesh, int startIndex, int endIndex)
{
	// Todo: add index selection support

	if(!isBound())
		throw std::exception("Mesh must be bound before updating");

	if(meshPtr->getByteSize() != mesh.getByteSize())
		throw std::exception("The new mesh must be of the same size");

	bufferMeshBlock(mesh);
}

void MeshBuffer::draw(GLenum drawMode)
{
	ShaderProgram *sp = getActiveShader();
	if(!sp)
		throw std::exception("A shader must be active before drawing");
	
	if(meshPtr->getIndexCount() == 0)
		return;
		
	if(hasVao())
	{
		vao.bind();
		glDrawElements(drawMode, meshPtr->getIndexCount(), GL_UNSIGNED_INT, 0);
		vao.unbind();
	}
	else if(interleavedVertexData)
	{
		vbo.bind();
		ibo.bind();
		int offset = 0;

		// This can be moved outside, in a batched mesh renderer
		if(meshPtr->getPositionCount())
		{
			sp->setAttributefv("position", 3, stride, offset); 
			offset += 3;
		}
		if(meshPtr->getNormalCount())
		{
			sp->setAttributefv("normal", 3, stride, offset); 
			offset += 3;
		}
		if(meshPtr->getColorCount())
		{
			sp->setAttributefv("color", 4, stride, offset); 
			offset += 4;
		}
		if(meshPtr->getTexelCount())
		{
			sp->setAttributefv("texel", 2, stride, offset);
			offset += 2;
		}
		if(meshPtr->getTangentsCount())
		{
			sp->setAttributefv("tangent", 3, stride, offset);
			sp->setAttributefv("bitangent", 3, stride, offset + 3);
		}

		glDrawElements(drawMode, meshPtr->getIndexCount(), GL_UNSIGNED_INT, 0);
		vbo.unbind();
		ibo.unbind();
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
	throw std::exception("VAO setup not done");

	ShaderProgram *shader = getActiveShader();
	if(!shader)
		throw std::exception("A shader must be active before VAO setup");

	vaoOk = true;
	vao.create();
	vao.bind();
	vbo.bind();
	ibo.bind();

	int offset = 0;
	if(meshPtr->getPositionCount() > 0)
	{
		shader->setAttributefv("position", 3, 0, offset);
		offset += meshPtr->getPositionCount() * 3;
	}
	if(meshPtr->getNormalCount() > 0)
	{
		shader->setAttributefv("normal", 3, 0, offset);
		offset += meshPtr->getNormalCount() * 3;
	}
	if(meshPtr->getColorCount() > 0)
	{
		shader->setAttributefv("color", 4, 0, offset);
		offset += meshPtr->getColorCount() * 4;
	}
	if(meshPtr->getTexelCount() > 0)
	{
		shader->setAttributefv("texel", 2, 0, offset);
		offset += meshPtr->getTexelCount() * 2;
	}

	vao.unbind();
	vbo.unbind();
	ibo.unbind();
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

// Deprecated

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