return function()
	warnings 'Off'
	includedirs {
		'third_party/glfw/include',
	}
	files {
		'third_party/glfw/src/egl_*.c',
		'third_party/glfw/src/osmesa_*.c',
		'third_party/glfw/src/context.c',
		'third_party/glfw/src/init.c',
		'third_party/glfw/src/input.c',
		'third_party/glfw/src/monitor.c',
		'third_party/glfw/src/vulkan.c',
		'third_party/glfw/src/window.c',
	}

	filter 'system:windows'
	defines {
		'_GLFW_WIN32',
	}
	files {
		'third_party/glfw/src/win32_*.c',
		'third_party/glfw/src/wgl_*.c',
	}

	filter 'system:linux'
	defines {
		'_GLFW_X11'
	}
	files {
		'third_party/glfw/src/glx_*.c',
		'third_party/glfw/src/linux_*.c',
		'third_party/glfw/src/posix_*.c',
		'third_party/glfw/src/x11_*.c',
		'third_party/glfw/src/xkb_*.c',
	}

	filter 'system:macosx'
	defines {
		'_GLFW_COCOA',
	}
	files {
		'third_party/glfw/src/cocoa_*.c',
		'third_party/glfw/src/posix_*.c',
	}
end
