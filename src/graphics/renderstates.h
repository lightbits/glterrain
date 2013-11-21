#ifndef RENDER_STATES_H
#define RENDER_STATES_H
#include <gl/glutils.h>

/*
Various renderstates that can be used in conjuction with the GL renderer.
Example:
Renderer.setBlendState(BlendStates::AlphaBlend);
Renderer.setRasterizerState(RasterizerStates::CullClockwise);

Inspired by XNA http://iloveshaders.blogspot.no/2011/04/using-state-objects-in-xna.html
*/

/*
Pixels can be drawn using a function that blends the incoming (source) RGBA values with the RGBA values that are already in the frame buffer (the destination values).
*/
struct BlendState
{
	BlendState(bool enabled = false, GLenum sourceBlend = GL_ONE, GLenum destinationBlend = GL_ZERO, GLenum blendEquation = GL_ADD) : 
		Enabled(enabled), SourceBlend(sourceBlend), DestinationBlend(destinationBlend), BlendEquation(blendEquation) { }
	bool Enabled;
	GLenum SourceBlend; // Source factor
	GLenum DestinationBlend; // Destination factor
	GLenum BlendEquation; // Specifies how source and destination colors are combined
	void set()
	{
		if(Enabled)
		{
			glEnable(GL_BLEND);
			glBlendFunc(SourceBlend, DestinationBlend);
			glBlendEquation(BlendEquation);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
};

/*
The incoming pixel depth value can be compared with the depth value present in the depth buffer,
to determine whether the pixel shall be drawn or not.
*/
struct DepthTestState
{
	DepthTestState(bool enabled = false, GLenum depthTestFunc = GL_LESS) : Enabled(enabled), DepthTestFunc(depthTestFunc) { }
	GLenum Enabled;
	GLenum DepthTestFunc;
	void set()
	{
		if(Enabled)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(DepthTestFunc);
			glDepthRange(0.0, 1.0);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
		}
	}
};

/*
Triangles are either drawn in a clockwise or counterclockwise order. Facets that face away from the viewer
can be hidden by setting the rasterizer state to cull such facets.
*/
struct CullState
{
	CullState(bool enabled = false, GLenum frontFace = GL_CCW, GLenum cullFace = GL_BACK) : 
		Enabled(enabled), FrontFace(frontFace), CullFace(cullFace) { }
	bool Enabled;
	GLenum FrontFace;
	GLenum CullFace;
	void set()
	{
		if(Enabled)
		{
			glEnable(GL_CULL_FACE);
			glFrontFace(FrontFace);
			glCullFace(CullFace);	
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}
};

/*
Polygons can either be drawn as filled triangles, or a variety of other modes, such as lines.
*/
struct RasterizerState
{
	RasterizerState(GLenum face = GL_FRONT_AND_BACK, GLenum mode = GL_FILL) : 
		Face(face), Mode(mode) { }
	GLenum Face;
	GLenum Mode;
	void set()
	{
		glPolygonMode(Face, Mode);
	}
};

/*
Enable or disable GL_TEXTURE(n)D and mipmap levels.
With fixed pipeline, you needed to call glEnable(GL_TEXTURE_2D) to enable 2D texturing. You needed to call glEnable(GL_LIGHTING). Since shaders override these functionalities, you don't need to glEnable/glDisable. If you don't want texturing, you either need to write another shader that doesn't do texturing or you can attach a all white or all black texture, depending on your needs. You can also write one shader that does lighting and one that doesn't.

http://www.opengl.org/wiki/GLSL_:_common_mistakes
*/
// struct SamplerState
// {
// 	SamplerState(bool enabled = false, GLenum textureMode) : Enabled(enabled), TextureMode(textureMode) { }
// 	bool Enabled;
// 	GLenum TextureMode;
// 	// Mipmap, what ever
// };

/*
See http://www.andersriggelsen.dk/glblendfunc.php and
http://www.learnopengles.com/android-lesson-five-an-introduction-to-blending/
*/
namespace BlendStates
{
	static const BlendState Default = BlendState(false);
	static const BlendState Opaque = BlendState(false);
	static const BlendState AlphaBlend = BlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
	static const BlendState Additive = BlendState(true, GL_ONE, GL_ONE, GL_FUNC_ADD);
	static const BlendState Multiplicative = BlendState(true, GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD);
	static const BlendState Interpolative = BlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
}

namespace CullStates
{
	static const CullState Default = CullState(false);
	static const CullState CullNone = CullState(false);
	static const CullState CullClockwise = CullState(true, GL_CW, GL_BACK);
	static const CullState CullCounterClockwise = CullState(true, GL_CCW, GL_BACK);
}

namespace RasterizerStates
{
	static const RasterizerState Default = RasterizerState();
	static const RasterizerState FillBoth = RasterizerState();
	static const RasterizerState FillFront = RasterizerState(GL_FRONT, GL_FILL);
	static const RasterizerState FillBack = RasterizerState(GL_BACK, GL_FILL);
	static const RasterizerState LineBoth = RasterizerState(GL_FRONT_AND_BACK, GL_LINE);
	static const RasterizerState LineFront = RasterizerState(GL_FRONT, GL_LINE);
	static const RasterizerState LineBack = RasterizerState(GL_BACK, GL_LINE);
}

namespace DepthTestStates
{
	static const DepthTestState Default = DepthTestState(false);
	static const DepthTestState Never = DepthTestState(true, GL_NEVER);
	static const DepthTestState Less = DepthTestState(true, GL_LESS);
	static const DepthTestState Equal = DepthTestState(true, GL_EQUAL);
	static const DepthTestState LessThanOrEqual = DepthTestState(true, GL_LEQUAL);
	static const DepthTestState Greater = DepthTestState(true, GL_GREATER);
	static const DepthTestState NotEqual = DepthTestState(true, GL_NOTEQUAL);
	static const DepthTestState GreaterThanOrEqual = DepthTestState(true, GL_GEQUAL);
	static const DepthTestState Always = DepthTestState(true, GL_ALWAYS);
}

// namespace SamplerStates
// {
// 	// static const SamplerState Default = SamplerState()
// }

#endif