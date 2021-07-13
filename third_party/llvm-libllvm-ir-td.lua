return function()
	kind 'Utility'
	configmap {
		[ 'Debug' ] = 'Release',
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
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} --gen-attrs -o %{file.directory}/Attributes.inc',
		}
		buildoutputs {
			'%{file.directory}/Attributes.inc',
		}

	filter 'files:third_party/llvm-project/llvm/include/llvm/IR/Intrinsics.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on IR/Intrinsics.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums -o %{file.directory}/IntrinsicEnums.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-impl -o %{file.directory}/IntrinsicImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=aarch64 -o %{file.directory}/IntrinsicsAArch64.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=amdgcn -o %{file.directory}/IntrinsicsAMDGPU.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=arm -o %{file.directory}/IntrinsicsARM.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=bpf -o %{file.directory}/IntrinsicsBPF.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=hexagon -o %{file.directory}/IntrinsicsHexagon.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=mips -o %{file.directory}/IntrinsicsMips.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=nvvm -o %{file.directory}/IntrinsicsNVPTX.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=ppc -o %{file.directory}/IntrinsicsPowerPC.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=r600 -o %{file.directory}/IntrinsicsR600.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=riscv -o %{file.directory}/IntrinsicsRISCV.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=s390 -o %{file.directory}/IntrinsicsS390.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=ve -o %{file.directory}/IntrinsicsVE.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=wasm -o %{file.directory}/IntrinsicsWebAssembly.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=x86 -o %{file.directory}/IntrinsicsX86.h',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-intrinsic-enums --intrinsic-prefix=xcore -o %{file.directory}/IntrinsicsXCore.h',
		}
		buildoutputs {
			'%{file.directory}/IntrinsicEnums.inc',
			'%{file.directory}/IntrinsicImpl.inc',
			'%{file.directory}/IntrinsicsAArch64.h',
			'%{file.directory}/IntrinsicsAMDGPU.h',
			'%{file.directory}/IntrinsicsARM.h',
			'%{file.directory}/IntrinsicsBPF.h',
			'%{file.directory}/IntrinsicsHexagon.h',
			'%{file.directory}/IntrinsicsMips.h',
			'%{file.directory}/IntrinsicsNVPTX.h',
			'%{file.directory}/IntrinsicsPowerPC.h',
			'%{file.directory}/IntrinsicsR600.h',
			'%{file.directory}/IntrinsicsRISCV.h',
			'%{file.directory}/IntrinsicsS390.h',
			'%{file.directory}/IntrinsicsVE.h',
			'%{file.directory}/IntrinsicsWebAssembly.h',
			'%{file.directory}/IntrinsicsX86.h',
			'%{file.directory}/IntrinsicsXCore.h',
		}
end
