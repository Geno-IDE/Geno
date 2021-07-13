return function()
	kind 'Utility'
	configmap {
		[ 'Debug' ] = 'Release',
	}
	dependson {
		'llvm-tablegen',
	}
	files {
		'third_party/llvm-project/llvm/include/llvm/Frontend/OpenMP/OMP.td',
	}

	filter 'files:third_party/llvm-project/llvm/include/llvm/Frontend/OpenMP/OMP.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Frontend/OpenMP/OMP.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-impl -o %{file.directory}/../../../../lib/Frontend/OpenMP/OMP.cpp',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-decl -o %{file.directory}/OMP.h.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-gen -o %{file.directory}/OMP.inc',
		}
		buildoutputs {
			'%{file.directory}/../../../../lib/Frontend/OpenMP/OMP.cpp',
			'%{file.directory}/OMP.h.inc',
			'%{file.directory}/OMP.inc',
		}
end
