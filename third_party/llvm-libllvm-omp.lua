return function()
	dependson {
		'llvm-libllvm-omp-td',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	files {
		'third_party/llvm-project/llvm/lib/Frontend/OpenMP/**.cpp',
		'third_party/llvm-project/llvm/lib/Frontend/OpenMP/OMP.cpp',
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
