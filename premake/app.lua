
require 'library'
require 'settings'

apps = { }

function app( name )
	group 'Apps'
	project( name )

	--appid '%{settings.bundle_namespace}.%{string.lower(prj.name)}'
	kind 'WindowedApp'
	links( libraries )
	location 'build/%{_ACTION}'
	sysincludedirs { 'include' }
	xcodebuildresources 'assets'

	files {
		'src/'..name..'/**.cpp',
		'src/'..name..'/**.h',
	}

	filter { 'system:linux' }
		linkoptions { '-Wl,-rpath=\\$$ORIGIN' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		compileas 'Objective-C++'

	filter { 'system:ios' }
		files { 'res/Info.plist', 'assets' }

	filter { 'system:android' }
		--files { 'src/Android/**' }
		--assetdirs { 'assets/' }

	filter { }

	table.insert( apps, name )
end
