
require 'library'
require 'settings'

apps = { }

function app( name )
	group 'Apps'
	project( name )

	androidmanifest 'src/%{prj.name}/AndroidManifest.xml'
	appid '%{settings.bundle_namespace}.%{string.lower(prj.name)}'
	assetdirs { 'src/%{prj.name}/Assets' }
	javadirs { 'src/%{prj.name}/Java' }
	kind 'WindowedApp'
	links( libraries )
	location 'build/%{_ACTION}'
	resdirs { 'src/%{prj.name}/Resources' }
	sysincludedirs { 'include' }
	xcodebuildresources 'src/%{prj.name}/Assets'

	files {
		'src/%{prj.name}/**.cpp',
		'src/%{prj.name}/**.h',
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
