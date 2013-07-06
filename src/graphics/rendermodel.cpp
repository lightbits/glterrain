#include <graphics/rendermodel.h>
using namespace graphics;

DynamicRenderModel::DynamicRenderModel() : vbo(), ibo(), fmt(), indexCount(0)
{
	vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	ibo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
}

void DynamicRenderModel::create(const TriMesh &mesh, const ShaderLayout &layout)
{
	fmt.clear();

	vbo.bind();

	vbo.bufferData(mesh.getByteSize(), NULL);
	GLintptr offset = 0;

	if(mesh.positions.size() > 0)
	{
		fmt.addAttrib(layout.positionAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getPositionsByteSize(), &mesh.positions[0]);
		offset += mesh.getPositionsByteSize();
	}

	if(mesh.normals.size() > 0)
	{
		fmt.addAttrib(layout.normalAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getNormalsByteSize(), &mesh.normals[0]);
		offset += mesh.getNormalsByteSize();
	}

	if(mesh.colors.size() > 0)
	{
		fmt.addAttrib(layout.colorAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getColorsByteSize(), &mesh.colors[0]);
		offset += mesh.getColorsByteSize();
	}

	if(mesh.texCoords.size() > 0)
	{
		fmt.addAttrib(layout.texelAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, offset);
		vbo.bufferSubData(offset, mesh.getTexCoordsByteSize(), &mesh.texCoords[0]);
	}

	indexCount = mesh.getIndexCount();
	ibo.bind();
	ibo.bufferData(indexCount * sizeof(GLushort), &mesh.indices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DynamicRenderModel::dispose()
{
	vbo.dispose();
	ibo.dispose();
}

void DynamicRenderModel::draw()
{
	vbo.bind();
	ibo.bind();
	fmt.enable(); // We would be able to skip alot of these if we used an interlaced vertex format and batched the models
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
	fmt.disable();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}