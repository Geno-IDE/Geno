return function()
	kind 'Utility'
	configmap {
		Debug = 'Release'
	}
	dependson {
		'llvm-tablegen',
	}
	files {
		'third_party/llvm-project/llvm/include/llvm/IR/Attributes.td',
		'third_party/llvm-project/llvm/include/llvm/IR/Intrinsics.td',
	}

	filter 'files:third_party/llvm-project/llvm/include/llvm/IR/Attributes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on IR/Attributes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} --gen-attrs -o %{wks.location}/third_party/.tablegen/llvm/IR/Attributes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/llvm/IR/Attributes.inc',
		}

	filter 'files:third_party/llvm-project/llvm/include/llvm/IR/Intrinsics.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on IR/Intrinsics.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicEnums.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-impl -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=aarch64 -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsAArch64.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=amdgcn -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsAMDGPU.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=arm -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsARM.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=bpf -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsBPF.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=hexagon -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsHexagon.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=mips -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsMips.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=nvvm -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsNVPTX.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=ppc -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsPowerPC.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=r600 -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsR600.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=riscv -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsRISCV.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=s390 -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsS390.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=ve -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsVE.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=wasm -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsWebAssembly.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=x86 -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsX86.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=xcore -o %{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsXCore.h',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicEnums.inc',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicImpl.inc',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsAArch64.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsAMDGPU.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsARM.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsBPF.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsHexagon.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsMips.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsNVPTX.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsPowerPC.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsR600.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsRISCV.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsS390.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsVE.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsWebAssembly.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsX86.h',
			'%{wks.location}/third_party/.tablegen/llvm/IR/IntrinsicsXCore.h',
		}
end
