return function()
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/llvm/lib/Demangle/**.cpp',
		'third_party/llvm-project/llvm/lib/Support/**.c',
		'third_party/llvm-project/llvm/lib/Support/**.cpp',
		'third_party/llvm-project/llvm/lib/TableGen/**.cpp',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
