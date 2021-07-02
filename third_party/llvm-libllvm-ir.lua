return function()
	dependson {
		'llvm-tablegen',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	prebuildcommands {
		tablegen( 'IR/Attributes.inc',          'IR/Attributes.td', '--gen-attrs -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicEnums.inc',      'IR/Intrinsics.td', '--gen-intrinsic-enums -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicImpl.inc',       'IR/Intrinsics.td', '--gen-intrinsic-impl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsAArch64.h',     'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=aarch64 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsAMDGPU.h',      'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=amdgcn -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsARM.h',         'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=arm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsBPF.h',         'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=bpf -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsHexagon.h',     'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=hexagon -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsMips.h',        'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=mips -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsNVPTX.h',       'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=nvvm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsPowerPC.h',     'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=ppc -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsR600.h',        'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=r600 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsRISCV.h',       'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=riscv -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsS390.h',        'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=s390 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsVE.h',          'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=ve -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsWebAssembly.h', 'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=wasm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsX86.h',         'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=x86 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsXCore.h',       'IR/Intrinsics.td', '--gen-intrinsic-enums --intrinsic-prefix=xcore -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
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
