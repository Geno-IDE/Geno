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

function tablegen( output, input, flags )
	return 'echo TableGen on '..output..' && "%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-tablegen")}" "%{path.join(wks.location,"third_party/llvm-project/llvm/include/llvm/'..input..'")}" '..flags..' -o "%{path.join(wks.location,"third_party/llvm-project/llvm/include/llvm/'..output..'")}"'
end

function clang_tablegen( output, input, flags )
	return 'echo Clang TableGen on '..output..' && "%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-clang-tablegen")}" "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/'..input..'")}" '..flags..' -o "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/'..output..'")}"'
end

third_party_library 'glew'
third_party_library 'glfw'
third_party_library 'imgui'
third_party_library 'discordrpc'
third_party_library 'llvm-clang-tablegen'
third_party_library 'llvm-libclang'
third_party_library 'llvm-libllvm'
third_party_library 'llvm-libllvm-target'
third_party_library 'llvm-tablegen'

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
		'third_party/llvm-project/clang/include',
		'third_party/stb',
		'third_party/rapidjson/include',
		'third_party/discord-rpc/include',
	}

	filter 'system:windows'
		links {
			'shell32',
			'gdi32',
			'Version',
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
