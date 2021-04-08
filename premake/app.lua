
require 'library'
require 'third_party_library'

apps = { }

function app( name )
	group 'Apps'
	project( name )

	debugdir 'src/%{prj.name}/Assets'
	includedirs { 'src/%{prj.name}/C++' }
	kind 'WindowedApp'
	links( libraries )
	links( third_party_libraries )
	location 'build/%{_ACTION}'
	sysincludedirs { 'include' }
	xcodebuildresources 'src/%{prj.name}/Assets'

	files {
		'src/%{prj.name}/C++/**.cpp',
		'src/%{prj.name}/C++/**.h',
	}

	includedirs {
		'src/%{prj.name}/Resources',
	}

	vpaths {
		[ 'Source Files/*' ] = 'src/' .. name .. '/C++',
	}

	filter 'system:windows'
		files {
			'src/%{prj.name}/Resources/win32-icons.rc',
			'src/%{prj.name}/Resources/win32-resource.h',
		}
		vpaths {
			[ 'Resources/*' ] = 'src/' .. name .. '/Resources',
		}

	filter 'system:linux'
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }

	filter 'system:ios'
		files { 'src/%{prj.name}/Resources/Info.plist', 'src/%{prj.name}/Assets' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		compileas 'Objective-C++'

	filter { }

	table.insert( apps, name )
end
