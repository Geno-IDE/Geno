require 'premake/app'
require 'premake/customizations'
require 'premake/defaults'
require 'premake/library'
require 'premake/options'
require 'premake/target'
require 'premake/third_party_library'
require 'premake/utils'

workspace( 'Geno' )
platforms( utils.get_platforms() )
configurations { 'Debug', 'Release' }
startproject( 'Geno' )

third_party_library 'glew'
third_party_library 'glfw'
third_party_library 'imgui'

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
	}
	libdirs {
		'third_party/glew/lib/Release/%{cfg.platform}'
	}

	filter 'system:windows'
		links {
			'opengl32',
		}

	filter 'system:linux'
		links {
			'pthread',
			'dl',
			'GL',
			'X11',
		}
