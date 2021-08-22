return function()
	defines {
		'GLFW_RESIZE_NESW_CURSOR',
	}
	sysincludedirs {
		'third_party/glew/include',
		'third_party/glfw/include',
	}
	includedirs {
		'third_party/imgui',
	}
	files {
		'third_party/imgui/*.cpp',
		'third_party/imgui/backends/imgui_impl_glfw.cpp',
		'third_party/imgui/backends/imgui_impl_opengl3.cpp',
		'third_party/imgui/misc/cpp/*.cpp',
	}
end
