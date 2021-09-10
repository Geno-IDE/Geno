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

#include <filesystem>
#include <string>
#include <vector>
#include <chrono>

#include <imgui.h>

class StatusBar
{
	GENO_SINGLETON( StatusBar );

//////////////////////////////////////////////////////////////////////////

public:

	struct Message
	{
		std::string Msg        = "";
		int64_t     Timestamp  = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
		int         ExpiryTime = 5; // Time in seconds

	}; // Message

	enum class Color
	{
		DEFAULT,
		BLACK,
		BLUE,
		ORANGE,
		RED,

	}; // Color

	static constexpr float HEIGHT = 24.f;

//////////////////////////////////////////////////////////////////////////

	StatusBar( void ) = default;

//////////////////////////////////////////////////////////////////////////

public:

	void Show              ( void );
	void SetColor          ( int R, int G, int B );
	void SetColor          ( Color color );
	void SetText           ( std::string Text );
	void SetCurrentFileInfo( int Column, int Row, int Length, int Lines );

//////////////////////////////////////////////////////////////////////////

private:

	Message     m_Message      = { };

	std::string m_Text         = "Ready";
	std::string m_TextEditInfo = "";

	int         m_Col_R        = 48;
	int         m_Col_G        = 48;
	int         m_Col_B        = 48;
	bool        m_Active       = 0;

}; // StatusBar
