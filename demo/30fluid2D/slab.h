/* 
A slab is a pair of an input- and output-rendertarget.
These can represent grids of values, such as the velocity vector field,
or the scalar pressure field.
*/
#ifndef SLAB
#define SLAB
#include <gl/framebuffer.h>

//class Slab
//{
//public:
//	Slab(int width, int height, GLenum dataFormat, const GLvoid *data);
//	Surface &ping();
//	Surface &pong();
//private:
//	int input_index;
//	int output_index;
//	Surface surfaces[2];
//
//	Slab() : input_index(0), output_index(1) { }
//
//	void swapSurfaces()
//	{
//		input_index = output_index;
//		output_index = (output_index + 1) % 2;
//	}
//
//	Surface &ping() { return surfaces[input_index]; }
//	Surface &pong() { return surfaces[output_index]; }
//
//	Surface surfaces[2];
//};

#endif