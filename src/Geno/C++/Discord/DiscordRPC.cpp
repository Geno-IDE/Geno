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

#define GENO_DISCORD_APP_ID 873985189037097052       // Geno IDE discord app
#define GENO_DISCORD_APP_ID_STR "873985189037097052" // Geno IDE discord app

#include "DiscordRPC.h"
#include "Application.h"

#include <iostream>
#include <chrono>

static const auto s_StartTime = std::chrono::system_clock::now();
static int64_t s_StartInUnixTime = -1;

//////////////////////////////////////////////////////////////////////////

static const char* GetImgFromStr( const std::string& str )
{
	if( str == ".txt" )
		return "img_l_txt_format";
	else if( str == ".cpp" || str == ".hpp" || str == ".h" /* Assume .h is c++ */ )
		return "img_l_cpp_format";
	else if( str == ".c" )
		return "img_l_c_format";
	else if( str == ".cs" )
		return "img_l_cs_format"; // Just when (if) geno supports cs
	else if( str == "genoinrt" )
		return "img_s_geno";
	else if( str == ".gwks" )
		return "img_l_geno_wks_format";
	else if( str == ".gprj" )
		return "img_l_geno_prj_format";
	else
		return "img_l_default_format";
} // GetImgFromStr

//////////////////////////////////////////////////////////////////////////

// NOTE(beast) prb a memory leak here not sure

void GenoDiscord::UpdateDiscord( void )
{
	s_StartInUnixTime = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( s_StartTime.time_since_epoch() ).count();

#if OLD
	DiscordRichPresence discordPresence;
	memset( &discordPresence, 0, sizeof( discordPresence ) );
	discordPresence.state = m_CurrentRPC.state;
	discordPresence.details = m_CurrentRPC.details;
	discordPresence.startTimestamp = 150766587586;
	discordPresence.instance = 0;
	m_CurrentRPC = discordPresence;
	discordPresence ={};
#endif

	if( m_Settings.Show )
	{
		DiscordRichPresence DiscordPresence;
		memset( &DiscordPresence, 0, sizeof( DiscordPresence ) );

		if( m_Settings.ShowFilename )
			if( m_CurrentFile == "" || m_CurrentFile == "No File" )
				DiscordPresence.state = "No File";
			else if( m_CurrentFile != ( const char* )"No File" )
				DiscordPresence.state = m_CurrentFile.c_str();

		if( m_Settings.ShowWrksName )
			DiscordPresence.details = m_Workspace.c_str();

		if( m_Settings.ShowTime )
			DiscordPresence.startTimestamp  = ( int64_t )s_StartInUnixTime;

		DiscordPresence.instance = 0;

		DiscordPresence.largeImageKey = GetImgFromStr( m_CurrentFileExt );
		DiscordPresence.smallImageKey = GetImgFromStr( "genoinrt" );
		DiscordPresence.smallImageText = "Geno";

		m_CurrentRPC = DiscordPresence;

		DiscordPresence ={};

		Discord_UpdatePresence( &m_CurrentRPC );
		Discord_RunCallbacks();
	}

} // UpdateDiscord

//////////////////////////////////////////////////////////////////////////

static void handleDiscordReady( const DiscordUser* connectedUser )
{
	printf( "\nDiscord: connected to user %s#%s - %s\n",
		   connectedUser->username,
		   connectedUser->discriminator,
		   connectedUser->userId );
} // handleDiscordReady

static void handleDiscordDisconnected( int errcode, const char* message )
{
	printf( "\nDiscord: disconnected (%d: %s)\n", errcode, message );
} // handleDiscordDisconnected

static void handleDiscordError( int errcode, const char* message )
{
	printf( "\nDiscord: error (%d: %s)\n", errcode, message );
} // handleDiscordError

static void handleDiscordJoin( const char* scrt )
{
	// To stop error C2220
	scrt;
} // handleDiscordJoin

// We don't use these but we need it in order  for the discord rpc to work
static void handleDiscordSpectate( const char* scrt )
{
	// To stop error C2220
	scrt;
} // handleDiscordSpectate

static void handleDiscordJoinRequest( const DiscordUser* request )
{
	// To stop error C2220
	request;
} // handleDiscordJoinRequest

//////////////////////////////////////////////////////////////////////////

void GenoDiscord::InitDiscord( void )
{
	DiscordEventHandlers handlers;
	memset( &handlers, 0, sizeof( handlers ) );

	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	handlers.joinGame = handleDiscordJoin;
	handlers.spectateGame = handleDiscordSpectate;
	handlers.joinRequest = handleDiscordJoinRequest;

	Discord_Initialize( GENO_DISCORD_APP_ID_STR, &handlers, 1, NULL );
} // Init Discord

//////////////////////////////////////////////////////////////////////////

void GenoDiscord::Shutdown( void )
{
	Discord_Shutdown();
} // Shutdown
