-- see http://glsdk.sourceforge.net/docs/html/pg_use.html

solution "glterrain"
	configurations {"Debug", "Release"}

	configuration {"Debug"}
		targetdir "bin/debug"

	configuration {"Release"}
		targetdir "bin/release"

	project "glterrain"
		kind "ConsoleApp"
		language "c++"
		files {"**.cpp", "**.h"} -- recursively add files

		includedirs {"../../glm", "../../sdl/include", "../../soil/include", "../../glew/include", "./src"}
		libdirs {"../../sdl/lib/x86", "../../soil/lib", "../../glew/lib/x86"}

		configuration "windows"
			defines {"GLEW_STATIC", "WIN32"}
			links {"opengl32", "glew32s", "SDL2", "SDL2main", "SOIL"}

		configuration "Debug"
			targetsuffix "D"
			defines "_DEBUG"
			flags "Symbols"
			
		configuration "Release"
			defines "NDEBUG"
			flags {"OptimizeSpeed", "NoFramePointer", "ExtraWarnings", "NoEditAndContinue"}