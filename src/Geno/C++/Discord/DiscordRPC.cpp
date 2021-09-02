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

#include "DiscordRPC.h"

#include "Application.h"

#include <iostream>

#define DISCORD_APP_ID "883058757163159633"

//////////////////////////////////////////////////////////////////////////

// TODO: Make this better

static const char* GetImgFromStr( const std::string& str )
{
	if( str == ".cpp" || str == ".hpp" || str == ".h" /* Assume .h is c++ */ || str == ".cxx" )
		return "cpp";
	else if( str == ".c" )
		return "c";
	else if( str == ".cs" )
		return "csharp";
	else if( str == "genoinrt" )
		return "geno";
	else
		return "default";

} // GetImgFromStr

//////////////////////////////////////////////////////////////////////////

void DiscordRPC::UpdateDiscord( void )
{
	m_StartInUnixTime = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( m_StartTime.time_since_epoch() ).count();

	if( m_Settings.Show )
	{
		DiscordRichPresence DiscordPresence{};

		if( m_Settings.ShowFilename )
		{
			if( m_CurrentFile == "" || m_CurrentFile == "No File" )
			{
				DiscordPresence.state = "No File";
			}
			else if( m_CurrentFile != ( const char* )"No File" )
			{
				DiscordPresence.state = m_CurrentFile.c_str();
			}
		}

		if( m_Settings.ShowWrksName )
			DiscordPresence.details = m_Workspace.c_str();

		if( m_Settings.ShowTime )
			DiscordPresence.startTimestamp  = ( int64_t )m_StartInUnixTime;

		DiscordPresence.instance = 0;

		DiscordPresence.largeImageKey = GetImgFromStr( m_CurrentFileExt );
		DiscordPresence.smallImageKey = GetImgFromStr( "genoinrt" );
		DiscordPresence.smallImageText = "Geno";

		m_CurrentRPC = DiscordPresence;

		DiscordPresence ={};

		Discord_UpdatePresence( &m_CurrentRPC );
	}

	Discord_RunCallbacks();

} // UpdateDiscord

//////////////////////////////////////////////////////////////////////////

static void HandleDiscordReady( const DiscordUser* connectedUser )
{
	// TODO: Print to status bar
	printf( "\nDiscord: connected to user %s#%s - %s\n", connectedUser->username, connectedUser->discriminator, connectedUser->userId );

} // HandleDiscordReady

//////////////////////////////////////////////////////////////////////////

static void HandleDiscordDisconnected( int errcode, const char* message )
{
	// TODO: Print to status bar
	printf( "\nDiscord: disconnected (%d: %s)\n", errcode, message );

} // HandleDiscordDisconnected

//////////////////////////////////////////////////////////////////////////

static void HandleDiscordError( int errcode, const char* message )
{
	// TODO: Print to status bar
	printf( "\nDiscord: error (%d: %s)\n", errcode, message );

} // HandleDiscordError

//////////////////////////////////////////////////////////////////////////

void DiscordRPC::InitDiscord( void )
{
	DiscordEventHandlers handlers{};

	handlers.ready        = HandleDiscordReady;
	handlers.disconnected = HandleDiscordDisconnected;
	handlers.errored      = HandleDiscordError;

	Discord_Initialize( DISCORD_APP_ID, &handlers, 1, NULL );

} // Init Discord

//////////////////////////////////////////////////////////////////////////

void DiscordRPC::Shutdown( void )
{
	Discord_Shutdown();

} // Shutdown
