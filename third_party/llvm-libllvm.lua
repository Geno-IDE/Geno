return function()
	dependson {
		'llvm-libllvm-td',
		'llvm-libllvm-omp-td',
	}
	includedirs {
		'third_party/.tablegen',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/llvm/lib/IR/**.cpp',
		'third_party/llvm-project/llvm/lib/ProfileData/**.cpp',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
