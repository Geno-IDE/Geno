return function()
	dependson {
		'llvm-tablegen',
		'llvm-libllvm-ir',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	prebuildcommands {
		tablegen( '../../lib/Frontend/OpenMP/OMP.cpp', 'Frontend/OpenMP/OMP.td', '--gen-directive-impl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'Frontend/OpenMP/OMP.h.inc',         'Frontend/OpenMP/OMP.td', '--gen-directive-decl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'Frontend/OpenMP/OMP.inc',           'Frontend/OpenMP/OMP.td', '--gen-directive-gen -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
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
