OUTDIR    = 'build/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/'
BUNDLE    = 'com.gaztin.foundation'
WORKSPACE = 'Workspace'

local modules = { }
local samples = { }

-- Allow Objective C++ files on macOS and iOS
premake.api.addAllowed( 'language', 'ObjCpp' )

-- Set system to android
if( _ACTION == 'android-studio' ) then
	_TARGET_OS = 'android'
	system( 'android' )
end

if( _TARGET_OS == 'macosx' ) then
	newoption {
		trigger     = 'ios',
		description = 'Target iOS'
	}
	if( _OPTIONS[ 'ios' ] ) then
		_TARGET_OS = 'ios'
	end
end

local function get_platforms()
	if( _ACTION == 'android-studio' ) then
		return { }
	elseif( os.host() == 'windows' ) then
		return os.is64bit() and { 'x64', 'x86' } or { 'x86' }
	else
		return { os.outputof( 'uname -m' ) }
	end
end

local function base_config()
	location( 'build/%{_ACTION}/' )
	objdir( OUTDIR )
	targetdir( OUTDIR )
	debugdir( 'assets' )
	cppdialect( 'C++17' )
	warnings( 'Extra' )
	rtti( 'Off' )
	exceptionhandling( 'Off' )
	--minsdkversion( '23' )
	--maxsdkversion( '28' )
	includedirs { 'src/' }
	sysincludedirs { 'src/' }
	flags { 'MultiProcessorCompile' }

	filter { 'configurations:Debug' }
		optimize( 'Off' )
		symbols( 'On' )

	filter { 'configurations:Release' }
		optimize( 'Full' )
		symbols( 'Off' )
		defines { 'NDEBUG' }

	filter { 'system:windows' }
		toolset( 'msc' )
		defines { 'NOMINMAX' }

	filter { 'system:not windows' }
		toolset( 'gcc' )

	filter { 'system:linux' }
		debugenvs { 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../%{OUTDIR}' }

	filter { 'system:android' }
		buildoptions { '-Wfatal-errors' }
		--androidabis { 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64' }

	filter { }
end

local function foreach_system( functor )
	local field = premake.field.get( 'system' )
	for i=1,#field.allowed do
		functor( field.allowed[ i ] )
	end
	functor( 'android' )
end

local function decl_module( name )
	local up = name:upper()

	group( 'Engine' )
	project( name )
	kind( 'StaticLib' )
	links( modules )
	--appid( '%{BUNDLE}.libs.' .. name:lower() )
	base_config()
	files {
		--'src/*.cpp',
		--'src/*.h',
	}

	filter { 'toolset:msc' }
		defines { '_CRT_SECURE_NO_WARNINGS' }

	filter { 'system:macosx or ios' }
		--files { 'src/**.mm' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		language( 'ObjCpp' )

	filter { }

	project()
	group()

	table.insert( modules, name )
end

local function decl_sample( name )
	local id       = string.format( '%02d', 1 + #samples )
	local fullname = id .. '-' .. name

	group( 'Samples' )
	project( fullname )
	kind( 'WindowedApp' )
	links( modules )
	xcodebuildresources( 'assets' )
	--appid( '%{BUNDLE}.samples.' .. name:lower() )
	base_config()
	files {
		--'src/*.cpp',
		--'src/*.h',
	}

	filter { 'system:linux' }
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		language( 'ObjCpp' )

	filter { 'system:ios' }
		files { 'res/Info.plist', 'assets' }

	filter { 'system:android' }
		--files { 'src/Android/**' }
		--assetdirs { 'assets/' }

	filter { }

	project()
	group()

	table.insert( samples, fullname )
end

workspace( WORKSPACE )
	platforms( get_platforms() )
	configurations { 'Debug', 'Release' }
	--gradleversion( '3.1.4' )

-- Declare modules
decl_module( 'Core' )

-- Declare samples
decl_sample( 'Tests' )

-- Set last sample to startup
workspace( WORKSPACE )
	startproject( samples[ #samples ] )
