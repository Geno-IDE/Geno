return function()
	sysincludedirs {
		'third_party/rapidjson/include',
	}

	includedirs {
		'third_party/discordrpc/include/Discord',
	}

	files {
		'third_party/discordrpc/src/**.cpp',
	}

	filter 'system:macosx'
		files {
			'third_party/discordrpc/src/**.m',
		}

		defines {
			'_DISCORD_MACOSX',
		}

	filter 'system:unix'
		defines {
			'_DISCORD_UNIX',
		}	

	filter 'system:linux'
		defines {
			'_DISCORD_LINUX',
		}	

	filter 'system:windows'
		defines {
			'_DISCORD_WINDOWS',
		}	

end
