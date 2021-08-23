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

// TODO: Move into StatusBar class?
struct StatusBarMessage
{
	std::string Message = "";
	int64_t Timestamp = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
	int ExpiryTime = 5; // Time in seconds
	int ExpiryTimeCurrent = 0;
}; // StatusBarMessage

class StatusBar
{
	GENO_SINGLETON( StatusBar )
public:
	StatusBar  ( void ) = default;
	~StatusBar ( void );

	//////////////////////////////////////////////////////////////////////////

public:
	void Show ( bool* pOpen );
	void Init ( void );

	void SetColor( float r, float g, float b );
	void SetColor( ImVec4 color );

	void SetText( std::string txt );
	void SetText( const char* txt );
	void SetTextOnce( const char* txt );

	void Clear() { m_Text = "Ready"; }

	//////////////////////////////////////////////////////////////////////////

	int&       Height()       { return m_Height; }
	const int& Height() const { return m_Height; }

	int&       Width()        { return m_Width; }
	const int& Width() const  { return m_Width; }

private:
	int         m_Height               = 0;
	int         m_Width                = 0;
	bool        m_Active               = 0;
	std::string m_Text                 = "Ready";
	ImVec4      m_Color;
	std::vector<StatusBarMessage> m_Messages;

}; // StatusBar
