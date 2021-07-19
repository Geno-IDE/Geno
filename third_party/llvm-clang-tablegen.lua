return function()
	kind 'ConsoleApp'
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/clang/utils/TableGen/**.cpp',
	}
	links {
		'llvm-libllvm',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
		}

	filter 'system:linux'
		links {
			'pthread',
			'ncurses',
		}

	filter 'system:macosx'
		links {
			'ncurses',
		}
end
