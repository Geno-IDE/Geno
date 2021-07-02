return function()
	dependson {
		'llvm-tablegen',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
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
