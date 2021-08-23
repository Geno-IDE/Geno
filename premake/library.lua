
libraries = { }

function library( name )
	group 'Libraries'
	project( name )

	kind 'StaticLib'
	location 'build/%{_ACTION}'

	files {
		'include/%{prj.name}/**.h',
		'src/%{prj.name}/**.cpp',
		'src/%{prj.name}/**.h',
	}

	vpaths {
		[ '*' ] = {
			'src/'     .. name,
			'include/' .. name,
		}
	}

	filter 'system:macosx or ios'
		files { 'src/%{prj.name}/**.mm' }

	filter { 'system:macosx or ios', 'files:**.cpp' }
		compileas 'Objective-C++'

	filter { }

	table.insert( libraries, name )
end
