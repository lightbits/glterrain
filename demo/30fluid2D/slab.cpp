#include "slab.h"

Slab::Slab() : input_index(0), output_index(1) 
{ }

void Slab::createSurfaces(int width, int height, int numComponents, const float *initialData)
{
	GLenum texFormat = GL_R32F;
	GLenum dataFormat = GL_RED;
	switch (numComponents)
	{
	case 2: dataFormat = GL_RG;		texFormat = GL_RG32F;	break;
	case 3: dataFormat = GL_RGB;	texFormat = GL_RGB32F;	break;
	case 4: dataFormat = GL_RGBA;	texFormat = GL_RGBA32F; break;
	}
	targets[0].create(0, texFormat, width, height, dataFormat, GL_FLOAT, initialData);
	targets[1].create(0, texFormat, width, height, dataFormat, GL_FLOAT, initialData);
	targets[0].getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	targets[1].getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void Slab::swapSurfaces()
{
	input_index = output_index;
	output_index = (output_index + 1) % 2;
}

RenderTexture &Slab::ping() 
{ 
	return targets[input_index]; 
}

RenderTexture &Slab::pong() 
{
	return targets[output_index]; 
}