
require 'customizations'

cdialect 'C11'
cppdialect 'C++20'
exceptionhandling 'Off'
flags { 'FatalWarnings', 'MultiProcessorCompile' }
includedirs { '../include/' }
objdir '../obj'
rtti 'Off'
targetdir '../%{iif(prj.kind == "StaticLib" or prj.kind == "SharedLib","lib","bin")}/%{cfg.platform}/%{cfg.buildcfg}'
warnings 'Extra'

defines {
	'GLEW_STATIC',
	'IMGUI_IMPL_OPENGL_LOADER_GLEW',
	'IMGUI_DEFINE_MATH_OPERATORS',
	'ImTextureID=unsigned int',
}

filter 'configurations:Debug'
	optimize 'Off'
	symbols 'On'
	defines { 'DEBUG' }

filter 'configurations:Release'
	optimize 'Full'
	symbols 'Off'
	defines { 'RELEASE', 'NDEBUG' }

filter 'system:windows'
	toolset 'msc'
	defines {
		'GLFW_EXPOSE_NATIVE_WIN32',
		'NOMINMAX',
		'_CRT_NONSTDC_NO_WARNINGS',
		'_CRT_SECURE_NO_WARNINGS',
		'_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING',
	}

filter 'system:not windows'
	cppdialect 'C++2a'
	toolset 'gcc'

filter 'system:linux'
	debugenvs { 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../%{OUTDIR}' }

filter { }
