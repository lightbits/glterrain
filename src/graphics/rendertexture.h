#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTUER_H
#include <gl/framebuffer.h>
#include <gl/texture.h>
#include <graphics/graphics.h>

class RenderTexture : public Graphics
{
public:
	void create(int width, int height);
	void dispose();
private:

};

#endif