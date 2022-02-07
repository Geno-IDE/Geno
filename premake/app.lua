
require 'library'
require 'third_party_library'

apps = { }

function app( name )
	group 'Apps'
	project( name )

	includedirs { 'src/%{prj.name}/C++' }
	kind 'WindowedApp'
	links( libraries )
	links( third_party_libraries )
	location 'build/%{_ACTION}'
	sysincludedirs { 'include' }
	xcodebuildresources 'data'

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
			[ 'Resource Files/*' ] = 'src/' .. name .. '/Resources',
		}

	filter 'system:linux'
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }

	filter 'system:ios'
		files { 'src/%{prj.name}/Resources/Info.plist' }

	filter 'system:macosx'
		files { 'src/%{prj.name}/C++/**.mm' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		compileas 'Objective-C++'

    filter { 'action:xcode4' }
        defines { 'BUILD_XCODE' }

	filter { }

	table.insert( apps, name )
end
