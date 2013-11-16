#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTUER_H
#include <graphics/framebuffer.h>
#include <graphics/texture.h>
#include <engine/graphics.h>

namespace graphics
{

class RenderTexture : public Graphics
{
public:
	void create(int width, int height);
	void dispose();
private:

};

}

#endif