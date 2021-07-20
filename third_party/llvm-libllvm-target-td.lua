return function()
	kind 'Utility'
	dependson {
		'llvm-tablegen',
	}
	files {
		'third_party/llvm-project/llvm/lib/Target/X86/X86.td',
	}

	filter 'files:third_party/llvm-project/llvm/lib/Target/X86/X86.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Target/X86/X86.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-matcher -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmMatcher.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-writer -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmWriter.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-writer --asmwriternum=1 -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmWriter1.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-callingconv -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenCallingConv.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-dag-isel -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenDAGISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-disassembler -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenDisassemblerTables.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-x86-EVEX2VEX-tables -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenEVEX2VEXTables.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-exegesis -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenExegesis.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-fast-isel -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenFastISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-global-isel -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenGlobalISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-instr-info --long-string-literals=false -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenInstrInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-register-bank -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenRegisterBank.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-register-info -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenRegisterInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-subtarget -o %{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenSubtargetInfo.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmMatcher.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmWriter.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenAsmWriter1.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenCallingConv.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenDAGISel.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenDisassemblerTables.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenEVEX2VEXTables.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenExegesis.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenFastISel.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenGlobalISel.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenInstrInfo.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenRegisterBank.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenRegisterInfo.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Target/X86/X86GenSubtargetInfo.inc',
		}
end
