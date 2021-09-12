return function()
	dependson {
		'llvm-libclang-td',
		'llvm-libllvm-ir-td',
		'llvm-libllvm-omp-td',
		'llvm-libllvm-target-td',
	}
	includedirs {
		'third_party/.tablegen',
		'third_party/.tablegen/clang/AST/Interp',
		'third_party/.tablegen/clang/Sema',
		'third_party/llvm-project/clang/lib/Basic',
		'third_party/llvm-project/clang/lib/Driver',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/clang/include',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/clang/lib/AST/**.cpp',
		'third_party/llvm-project/clang/lib/Analysis/*.cpp', -- No plugins
		'third_party/llvm-project/clang/lib/Basic/**.cpp',
		'third_party/llvm-project/clang/lib/Driver/**.cpp',
		'third_party/llvm-project/clang/lib/Edit/**.cpp',
		'third_party/llvm-project/clang/lib/Format/**.cpp',
		'third_party/llvm-project/clang/lib/Frontend/**.cpp',
		'third_party/llvm-project/clang/lib/Index/**.cpp',
		'third_party/llvm-project/clang/lib/Lex/**.cpp',
		'third_party/llvm-project/clang/lib/Parse/**.cpp',
		'third_party/llvm-project/clang/lib/Rewrite/**.cpp',
		'third_party/llvm-project/clang/lib/Sema/**.cpp',
		'third_party/llvm-project/clang/lib/Serialization/**.cpp',
		'third_party/llvm-project/clang/lib/Tooling/**.cpp',
		'third_party/llvm-project/clang/tools/libclang/**.cpp',
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
