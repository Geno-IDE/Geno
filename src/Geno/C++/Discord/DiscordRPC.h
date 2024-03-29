/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#pragma once
#include <Common/Macros.h>

#include <string>
#include <cstring>
#include <chrono>

#include <discord_rpc.h>

class DiscordRPC
{
	GENO_SINGLETON( DiscordRPC );

//////////////////////////////////////////////////////////////////////////

public:

	struct Settings 
	{
		bool ShowFilename = true;
		bool ShowWrksName = true;
		bool ShowTime     = true;
		bool Show         = true;

	}; // Settings

//////////////////////////////////////////////////////////////////////////

	DiscordRPC( void ) = default;

//////////////////////////////////////////////////////////////////////////

	void UpdateDiscord ( void );
	void InitDiscord   ( void );
	void Shutdown      ( void );

//////////////////////////////////////////////////////////////////////////

	DiscordRichPresence m_CurrentRPC         = { };
	Settings            m_Settings           = { };

	std::string         m_CurrentFile        = { };
	std::string         m_Workspace          = { };
	std::string         m_CurrentFileExt     = { };

//////////////////////////////////////////////////////////////////////////

private:

	const std::chrono::system_clock::time_point m_StartTime        = std::chrono::system_clock::now();
	int64_t                                     m_StartInUnixTime  = -1;

}; // DiscordRPC
