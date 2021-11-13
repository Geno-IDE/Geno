return function()
	warnings 'Off'
	includedirs {
		'third_party/glfw-fork/include',
	}
	files {
		'third_party/glfw-fork/src/egl_*.c',
		'third_party/glfw-fork/src/osmesa_*.c',
		'third_party/glfw-fork/src/context.c',
		'third_party/glfw-fork/src/init.c',
		'third_party/glfw-fork/src/input.c',
		'third_party/glfw-fork/src/monitor.c',
		'third_party/glfw-fork/src/vulkan.c',
		'third_party/glfw-fork/src/window.c',
	}

	filter 'system:windows'
	defines {
		'_GLFW_WIN32',
	}
	files {
		'third_party/glfw-fork/src/win32_*.c',
		'third_party/glfw-fork/src/wgl_*.c',
	}

	filter 'system:linux'
	defines {
		'_GLFW_X11',
		'_POSIX_C_SOURCE=199309L',
	}
	files {
		'third_party/glfw-fork/src/glx_*.c',
		'third_party/glfw-fork/src/linux_*.c',
		'third_party/glfw-fork/src/posix_*.c',
		'third_party/glfw-fork/src/x11_*.c',
		'third_party/glfw-fork/src/xkb_*.c',
	}

	filter 'system:macosx'
	defines {
		'_GLFW_COCOA',
	}
	files {
		'third_party/glfw-fork/src/cocoa_*.c',
		'third_party/glfw-fork/src/cocoa_*.m',
		'third_party/glfw-fork/src/nsgl_*.m',
		'third_party/glfw-fork/src/posix_*.c',
	}
	removefiles {
		'third_party/glfw-fork/src/posix_time.c',
	}
end
