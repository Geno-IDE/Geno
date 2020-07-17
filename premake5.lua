require 'third_party/premake-android-studio'
require 'premake/app'
require 'premake/customizations'
require 'premake/defaults'
require 'premake/library'
require 'premake/options'
require 'premake/settings'
require 'premake/target'
require 'premake/third_party_library'
require 'premake/utils'

workspace( settings.workspace_name )
platforms( utils.get_platforms() )
configurations { 'Debug', 'Release' }

third_party_library 'glfw'
third_party_library 'imgui'

app( settings.workspace_name )
	sysincludedirs {
		'third_party/glew/include',
		'third_party/glfw/include',
		'third_party/imgui',
	}
	libdirs {
		'third_party/glew/lib/Release/%{cfg.platform}'
	}

	filter 'system:windows'
	ignoredefaultlibraries {
		'LIBCMT',
	}
	disablewarnings {
		'4099', -- PDB 'foo.pdb' was not found with 'bar.lib(baz.obj)' or at '/output/dir/foo.pdb'; linking object as if no debug info
	}
	links {
		'opengl32',
		'glew32s',
	}

-- Set last app as startup
workspace( settings.workspace_name )
startproject( apps[ #apps ] )
