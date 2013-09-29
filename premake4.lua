-- see http://glsdk.sourceforge.net/docs/html/pg_use.html

dofile("../../glsdk/links.lua") -- Make sure this path is correct!
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

		UseLibs {"glfw", "glload", "glimage", "glm"}

		configuration "windows"
			defines "WIN32" -- WIN32
			links {"opengl32"} -- might have to link glu32.lib if old

		configuration "Debug"
			targetsuffix "D"
			defines "_DEBUG"
			flags "Symbols"

		configuration "Release"
			defines "NDEBUG"
			flags {"OptimizeSpeed", "NoFramePointer", "ExtraWarnings", "NoEditAndContinue"}