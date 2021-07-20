return function()
	kind 'Utility'
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
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-impl -o %{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.cpp',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-decl -o %{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.h.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-directive-gen -o %{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.cpp',
			'%{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.h.inc',
			'%{wks.location}/third_party/.tablegen/llvm/Frontend/OpenMP/OMP.inc',
		}
end
