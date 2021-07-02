return function()
	dependson {
		'llvm-libllvm-target-td',
	}
	includedirs {
		'third_party/llvm-project/llvm/lib/Target/X86',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/llvm/lib/Target/*.cpp',
		'third_party/llvm-project/llvm/lib/Target/X86/**.cpp',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
