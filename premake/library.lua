
libraries = { }

function library( name )
	local up = name:upper()

	group 'Libraries'
	project( name )

	--appid '%{settings.bundle_namespace}.libs.%{string.lower(prj.name)}'
	kind 'StaticLib'
	links( libraries )
	location 'build/%{_ACTION}'

	files {
		'src/'..name..'/**.cpp',
		'src/'..name..'/**.h',
	}

	filter 'system:macosx or ios'
		files { 'src/'..name..'/**.mm' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		compileas 'Objective-C++'

	filter { }

	table.insert( libraries, name )
end
