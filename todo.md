Resources
====
https://github.com/amongpixels/tripping-archer
http://www.reddit.com/r/gamedev/comments/1lv2s9/trippingarcher_my_procedural_terrain_design/
http://vacuumflowers.com/weblog/?p=347
http://oddlabs.com/download/terrain_generation.pdf
http://www.gamasutra.com/view/feature/3434/continuous_lod_terrain_meshing_.php
https://github.com/rohitnirmal/Procedural-Terrain/blob/master/src/main.cpp
http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
http://procworld.blogspot.co.uk/
http://gamedev.stackexchange.com/questions/53478/how-to-do-triplanar-texturing
http://blog.ccoder.co.uk/?page=6

General
====
*	Move mesh generators out from TriMesh class
*	Centralize rendering code to a Renderer class
*	Move bufferData(const TriMesh &mesh) out of BufferObject. This makes no sense if we are handling a target that isn't GL_ARRAY_BUFFER.
*	Switch to interlaced vertex formats, as this actually supports batching.
*	Come up with some clever way to handle shared attrib locations

*	Consider dropping the BufferObject class. It just causes confusion and doesn't really make things simpler.

*	Find out why gDEBugger doesn't detect glBindTexture or glDraw*
*	Use debug context http://www.altdevblogaday.com/2011/06/23/improving-opengl-error-messages/

SpriteBatch
====
Look at LÖVE for inspiration
https://github.com/mattdesl/lwjgl-basics/blob/master/src/mdesl/graphics/SpriteBatch.java
http://gamedev.stackexchange.com/questions/37423/sprite-batching-seems-slow
http://www.opengl.org/discussion_boards/showthread.php/176176-Fast-Sprites-and-VBOs
https://bitbucket.org/rude/love/src/edd946442f86d77fac8da437f9c12431738727e6/src/modules/graphics/opengl/Image.cpp?at=default
http://stackoverflow.com/questions/11706433/opengl-tile-rendering-most-efficient-way
http://gamedev.stackexchange.com/questions/19315/efficient-sprite-batching

Seperate buffers for attributes?
http://devmaster.net/forums/topic/15692-vertex-buffers-interleaved-or-separate/

*	Consider lessening the usage of glBufferSubData in SpriteBatch, and instead store vertex information in application memory,
before doing uploading it to the GPU when it's time to draw.
*	Let the user set the view transform and blend mode as parameters in begin()

Approach 1: bufferSubData
For each sprite, we upload a set of vertex data to the vertex buffer, already transformed.
If the sprite uses a different texture than the previous, we flush.
For each flush, we draw the currently added data in the buffer using a single draw call (glDrawArrays(GL_QUADS)).

Pros: Fewer draw calls, if sprites share textures
Cons: More buffer calls, slow for large sets of differently textured sprites

Approach 2: uniforms
We upload a single set of dummy vertex data (unit quad).
For each sprite, we set a model transformation uniform in the program, as well as the texcoords, and perform a draw call.

Pros: Fewer buffer calls, faster for sets of differently textured sprites
Cons: A draw call for each sprite

RenderModel
====
DynamicRenderModel should not have a vao. The vertex format should not be interlaced?
The vbo will be updated frequently, so spending time interlacing the attributes would be inefficient.

StaticRenderModel should interlace the vertex attributes, as this allows for easy batching (the vertex format
is the same). Might want to use VAOs.