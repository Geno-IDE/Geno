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

// Using win64_dyn state
#include <discord_rpc.h>
#include <Common/Macros.h>

 // Geno Discord here as discord is a namespace
class GenoDiscord
{
	GENO_SINGLETON( GenoDiscord )

	struct GenoDiscordSettings 
	{
		bool ShowFilename = true;
		bool ShowWrksName = true;
		bool ShowTime     = true;
		bool Show         = true;
	}; // GenoDiscordSettings

public:
	GenoDiscord( void )  = default;
	~GenoDiscord( void ) = default;

	//////////////////////////////////////////////////////////////////////////
public:
	void UpdateDiscord  ( void );
	void InitDiscord    ( void );
	void Shutdown       ( void );
	//////////////////////////////////////////////////////////////////////////
public:
	DiscordRichPresence m_CurrentRPC{};

	GenoDiscordSettings m_Settings;

	std::string m_CurrentFile;
	std::string m_Workspace;
	std::string m_CurrentFileExt;

protected:
private:
}; // GenoDiscord
