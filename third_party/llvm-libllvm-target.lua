return function()
	dependson {
		'llvm-libllvm-ir-td',
		'llvm-libllvm-target-td',
		'llvm-libllvm-ir-td',
	}
	includedirs {
		'third_party/.tablegen',
		'third_party/.tablegen/llvm/Target/X86',
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
