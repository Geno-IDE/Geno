return function()
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/llvm/lib/BinaryFormat/**.cpp',
		'third_party/llvm-project/llvm/lib/Bitstream/**.cpp',
		'third_party/llvm-project/llvm/lib/Demangle/**.cpp',
		'third_party/llvm-project/llvm/lib/MC/**.cpp',
		'third_party/llvm-project/llvm/lib/Option/**.cpp',
		'third_party/llvm-project/llvm/lib/Remarks/**.cpp',
		'third_party/llvm-project/llvm/lib/Support/**.c',
		'third_party/llvm-project/llvm/lib/Support/**.cpp',
		'third_party/llvm-project/llvm/lib/TableGen/**.cpp',
	}

	filter 'toolset:msc'
		buildoptions {
			'/bigobj',
		}
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
