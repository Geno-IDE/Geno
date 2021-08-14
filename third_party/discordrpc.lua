return function()
	sysincludedirs {
		'third_party/rapidjson/include',
	}

	includedirs {
		'third_party/discord-rpc/include/',
	}

	files {
		'third_party/discord-rpc/src/**.cpp',
	}

	removefiles {
		'third_party/discord-rpc/src/dllmain.cpp',
	}

	filter 'system:macosx'
		files {
			'third_party/discord-rpc/src/**.m',
		}

		removefiles {
			'third_party/discord-rpc/src/connection_unix.cpp',
			'third_party/discord-rpc/src/connection_win.cpp',
			'third_party/discord-rpc/src/discord_register_linux.cpp',
			'third_party/discord-rpc/src/discord_register_win.cpp',
		}

		defines {
			'_DISCORD_MACOSX',
		}

	filter 'system:unix'
		defines {
			'_DISCORD_UNIX',
		}	

		removefiles {
			'third_party/discord-rpc/src/**.m',
			'third_party/discord-rpc/src/connection_win.cpp',
			'third_party/discord-rpc/src/discord_register_win.cpp',
		}


	filter 'system:linux'
		defines {
			'_DISCORD_LINUX',
		}

		removefiles {
			'third_party/discord-rpc/src/**.m',
			'third_party/discord-rpc/src/connection_win.cpp',
			'third_party/discord-rpc/src/discord_register_win.cpp',
		}


	filter 'system:windows'
		defines {
			'_DISCORD_WINDOWS',
		}	

		removefiles {
			'third_party/discord-rpc/src/**.m',
			'third_party/discord-rpc/src/connection_unix.cpp',
			'third_party/discord-rpc/src/discord_register_linux.cpp',
		}


end
