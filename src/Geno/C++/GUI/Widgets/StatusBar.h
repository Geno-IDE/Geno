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
	GENO_SINGLETON( StatusBar )
public:

	struct StatusBarMessage
	{
		std::string Message = "";
		int64_t Timestamp = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
		int ExpiryTime = 5; // Time in seconds

	}; // StatusBarMessage

	enum class StatusBarColor
	{
		DEFAULT,
		BLACK,
		BLUE,
		ORANGE,
		RED
	}; // StatusBarColor

public:
	StatusBar  ( void ) = default;
	~StatusBar ( void );

	//////////////////////////////////////////////////////////////////////////

public:
	void Show();
	void Init( void );

	void SetColor( int r, int g, int b );
	void SetColor( StatusBarColor color );

	void SetText( std::string txt );

	void SetCurrentFileInfo( int column, int row, int length, int lines );
	
	void Clear() { m_Text = "Ready"; }

	//////////////////////////////////////////////////////////////////////////

	int&               Height()           { return m_Height; }
	const int&         Height() const     { return m_Height; }

	int&               Width()            { return m_Width;  }
	const int&         Width() const      { return m_Width;  }

	std::string&       Text()             { return m_Text;   }
	const std::string& Text() const       { return m_Text;   }


private:
	int              m_Height                   = 0;
	int              m_Width                    = 0;
	bool             m_Active                   = 0;
	std::string      m_Text                     = "Ready";
	std::string      m_TextEditInfo             = "";
	int              m_Col_R                    = 0;
	int              m_Col_G                    = 0;
	int              m_Col_B                    = 0;
	StatusBarMessage m_Message;

}; // StatusBar
