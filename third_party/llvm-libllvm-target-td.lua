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
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-matcher -o %{file.directory}/X86GenAsmMatcher.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-writer -o %{file.directory}/X86GenAsmWriter.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-asm-writer --asmwriternum=1 -o %{file.directory}/X86GenAsmWriter1.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-callingconv -o %{file.directory}/X86GenCallingConv.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-dag-isel -o %{file.directory}/X86GenDAGISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-disassembler -o %{file.directory}/X86GenDisassemblerTables.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-x86-EVEX2VEX-tables -o %{file.directory}/X86GenEVEX2VEXTables.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-exegesis -o %{file.directory}/X86GenExegesis.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-fast-isel -o %{file.directory}/X86GenFastISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-global-isel -o %{file.directory}/X86GenGlobalISel.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-instr-info --long-string-literals=false -o %{file.directory}/X86GenInstrInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-register-bank -o %{file.directory}/X86GenRegisterBank.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-register-info -o %{file.directory}/X86GenRegisterInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory} -I %{file.directory}/../../../include --gen-subtarget -o %{file.directory}/X86GenSubtargetInfo.inc',
		}
		buildoutputs {
			'%{file.directory}/X86GenAsmMatcher.inc',
			'%{file.directory}/X86GenAsmWriter.inc',
			'%{file.directory}/X86GenAsmWriter1.inc',
			'%{file.directory}/X86GenCallingConv.inc',
			'%{file.directory}/X86GenDAGISel.inc',
			'%{file.directory}/X86GenDisassemblerTables.inc',
			'%{file.directory}/X86GenEVEX2VEXTables.inc',
			'%{file.directory}/X86GenExegesis.inc',
			'%{file.directory}/X86GenFastISel.inc',
			'%{file.directory}/X86GenGlobalISel.inc',
			'%{file.directory}/X86GenInstrInfo.inc',
			'%{file.directory}/X86GenRegisterBank.inc',
			'%{file.directory}/X86GenRegisterInfo.inc',
			'%{file.directory}/X86GenSubtargetInfo.inc',
		}
end
