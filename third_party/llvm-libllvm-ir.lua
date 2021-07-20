return function()
	dependson {
		'llvm-libllvm-ir-td',
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
		-- TODO: Check which of these don't depend on any TableGen'ed files, and thus can be moved into llvm-libllvm
		'third_party/llvm-project/llvm/lib/Analysis/**.cpp',
		'third_party/llvm-project/llvm/lib/Bitcode/**.cpp',
		'third_party/llvm-project/llvm/lib/CodeGen/**.cpp',
		'third_party/llvm-project/llvm/lib/DebugInfo/**.cpp',
		'third_party/llvm-project/llvm/lib/IR/**.cpp',
		'third_party/llvm-project/llvm/lib/Object/**.cpp',
		'third_party/llvm-project/llvm/lib/ProfileData/**.cpp',
		'third_party/llvm-project/llvm/lib/TextAPI/**.cpp',
		'third_party/llvm-project/llvm/lib/Transforms/**.cpp',
	}
	removefiles {
		'third_party/llvm-project/llvm/lib/DebugInfo/PDB/DIA/**.cpp',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
