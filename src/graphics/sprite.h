#ifndef SPRITE_H
#define SPRITE_H
#include <common/matrix.h>
#include <graphics/color.h>
#include <gl/texture.h>

class Sprite
{
public:
	Sprite();

	void setTexture(const Texture2D &texture);
	void setColor(const Color &color);
	void setPosition(const vec2 &position);
	void setTransform(const mat4 &transform);
	void setSize(float width, float height);
	void setTextureCoordf(float ul, float ur, float vb, float vt);
	void setTextureCoordi(int x, int y, int width, int height);

	Texture2D *getTexture() const { return texture; }
	vec2 getPosition() const { return position; }
	vec2 getSize() const { return size; }
	mat4 getTransform() { if(transformOld) { updateTransform(); transformOld = false; } return transform; }
private:
	void updateTransform();
	vec2 position;
	vec2 size;
	Texture2D *texture;
	mat4 transform;
	bool transformOld;
};

#endif