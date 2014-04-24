GL Terrain
=========

Rendering procedurally generated terrain in OpenGL. It has actually snowballed into a some sort of rendering framework frankenstein monster -_-;
Neat examples can be found in the `demo` folder.

External dependencies
---------------------
* **GLEW** for GL function loading
* **SDL** for context handling
* **glm** for matrix math
* **SOIL** for image loading

Build (Windows)
---------------
Make sure the paths to include and library folders are correct inside the `premake4.lua` script, and run premake4 to create a project, e.g.:
```
cd D:/glterrain
premake4 vs2010
```
Open the project file with your favorite IDE and try compiling.
You might have to compile SOIL on your own, and use /NODEFAULTLIB:MSVCRT;LIBCMT on the linker.

Build (Linux)
-------------
Linux has not been tested yet. It should work though, somehow.