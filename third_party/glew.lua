return function()
	warnings 'Off'
	defines {
		'GLEW_BUILD',
	}
	includedirs {
		'third_party/glew/include',
	}
	sysincludedirs {
		'third_party/glew/include',
	}
	files {
		'third_party/glew/src/glew.c',
	}
end
