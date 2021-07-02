return function()
	dependson {
		'llvm-tablegen',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
	}
	prebuildcommands {
		tablegen( '../../lib/Frontend/OpenMP/OMP.cpp', 'Frontend/OpenMP/OMP.td', '--gen-directive-impl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/Attributes.inc',                 'IR/Attributes.td',       '--gen-attrs -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicEnums.inc',             'IR/Intrinsics.td',       '--gen-intrinsic-enums -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicImpl.inc',              'IR/Intrinsics.td',       '--gen-intrinsic-impl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsAArch64.h',            'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=aarch64 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsAMDGPU.h',             'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=amdgcn -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsARM.h',                'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=arm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsBPF.h',                'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=bpf -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsHexagon.h',            'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=hexagon -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsMips.h',               'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=mips -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsNVPTX.h',              'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=nvvm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsPowerPC.h',            'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=ppc -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsR600.h',               'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=r600 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsRISCV.h',              'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=riscv -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsS390.h',               'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=s390 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsVE.h',                 'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=ve -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsWebAssembly.h',        'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=wasm -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsX86.h',                'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=x86 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'IR/IntrinsicsXCore.h',              'IR/Intrinsics.td',       '--gen-intrinsic-enums --intrinsic-prefix=xcore -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
	}
	files {
		'third_party/llvm-project/llvm/lib/Frontend/OpenMP/**.cpp',
		'third_party/llvm-project/llvm/lib/Frontend/OpenMP/OMP.cpp',
		'third_party/llvm-project/llvm/lib/IR/**.cpp',
	}

	filter 'platforms:Win32 or x64'
		includedirs {
			'third_party/llvm-project/llvm/lib/Target/X86',
		}
		files {
			'third_party/llvm-project/llvm/lib/Target/X86/**.cpp',
		}
		prebuildcommands {
			tablegen( '../../lib/Target/X86/X86GenAsmMatcher.inc',         '../../lib/Target/X86/X86.td', '--gen-asm-matcher -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenAsmWriter.inc',          '../../lib/Target/X86/X86.td', '--gen-asm-writer -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenAsmWriter1.inc',         '../../lib/Target/X86/X86.td', '--gen-asm-writer --asmwriternum=1 -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenCallingConv.inc',        '../../lib/Target/X86/X86.td', '--gen-callingconv -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenDAGISel.inc',            '../../lib/Target/X86/X86.td', '--gen-dag-isel -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenDisassemblerTables.inc', '../../lib/Target/X86/X86.td', '--gen-disassembler -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenEVEX2VEXTables.inc',     '../../lib/Target/X86/X86.td', '--gen-x86-EVEX2VEX-tables -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenExegesis.inc',           '../../lib/Target/X86/X86.td', '--gen-exegesis -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenFastISel.inc',           '../../lib/Target/X86/X86.td', '--gen-fast-isel -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenGlobalISel.inc',         '../../lib/Target/X86/X86.td', '--gen-global-isel -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenInstrInfo.inc',          '../../lib/Target/X86/X86.td', '--gen-instr-info --long-string-literals=false -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenRegisterBank.inc',       '../../lib/Target/X86/X86.td', '--gen-register-bank -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenRegisterInfo.inc',       '../../lib/Target/X86/X86.td', '--gen-register-info -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
			tablegen( '../../lib/Target/X86/X86GenSubtargetInfo.inc',      '../../lib/Target/X86/X86.td', '--gen-subtarget -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}" -I "%{path.join(wks.location,"third_party/llvm-project/llvm/lib/Target/X86")}"' ),
		}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
