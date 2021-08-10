require 'premake/premake-geno'
require 'premake/app'
require 'premake/customizations'
require 'premake/defaults'
require 'premake/library'
require 'premake/options'
require 'premake/target'
require 'premake/third_party_library'
require 'premake/utils'

workspace( 'Geno' )
platforms( utils.get_platform() )
configurations { 'Debug', 'Release' }
startproject( 'Geno' )

third_party_library 'glew'
third_party_library 'glfw'
third_party_library 'imgui'
third_party_library 'discordrpc'

library 'Common'
	sysincludedirs {
		'third_party/glew/include',
	}

library 'GCL'

app( 'Geno' )
	sysincludedirs {
		'third_party/glew/include',
		'third_party/glfw/include',
		'third_party/imgui',
		'third_party/stb',
		'third_party/rapidjson/include',
		'third_party/discordrpc/include',
	}

	filter 'system:windows'
		links {
			'shell32',
			'gdi32',
			'opengl32',
		}

	filter 'system:linux'
		links {
			'stdc++fs',
			'pthread',
			'dl',
			'GL',
			'X11',
		}

	filter 'system:macosx'
		files {
			"data",
		}
		links {
			'CoreGraphics.framework',
			'IOKit.framework',
			'AppKit.framework',
			'OpenGL.framework',
		}
