/* 
A slab is a pair of an input- and output-rendertarget.
These can represent grids of values, such as the velocity vector field,
or the scalar pressure field.
*/
#ifndef SLAB
#define SLAB
#include <graphics/rendertexture.h>

class Slab
{
public:
	Slab();

	void createSurfaces(int width, int height, int numComponents, const float *initialData);
	void swapSurfaces();

	RenderTexture &ping();
	RenderTexture &pong();
private:
	int input_index;
	int output_index;

	RenderTexture targets[2];
};

#endif