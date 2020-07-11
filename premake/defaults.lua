
require 'customizations'

cppdialect 'C++17'
debugdir '../assets'
exceptionhandling 'Off'
flags { 'MultiProcessorCompile' }
--gradleversion( '3.1.4' )
includedirs { '../include/' }
--maxsdkversion '28'
--minsdkversion '23'
objdir '../obj/%{cfg.platform}/%{cfg.buildcfg}'
rtti 'Off'
targetdir '../%{iif(prj.kind == "StaticLib","lib","bin")}/%{cfg.platform}/%{cfg.buildcfg}'
warnings 'Extra'

filter 'configurations:Debug'
	optimize 'Off'
	symbols 'On'

filter 'configurations:Release'
	optimize 'Full'
	symbols 'Off'
	defines { 'NDEBUG' }

filter 'system:windows'
	toolset 'msc'
	defines { 'NOMINMAX' }

filter 'system:not windows'
	toolset 'gcc'

filter 'system:linux'
	debugenvs { 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../%{OUTDIR}' }

filter 'system:android'
	--androidabis { 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64' }

filter { }
