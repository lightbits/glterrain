#ifndef EFFECT_PASS_H
#define EFFECT_PASS_H
#include <graphics/program.h>
#include <graphics/shader.h>
namespace graphics
{

class EffectPass
{
public:
	EffectPass();
	bool loadFromFile(const std::string &vertName, const std::string &fragName, const std::string &geomName = "");
	void dispose();

	void begin();
	void end();
private:
	Program program;
	Shader vertexShader;
	Shader fragmentShader;
};

}

#endif