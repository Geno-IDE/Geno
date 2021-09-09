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

#include "StatusBar.h"
#include "GUI/PrimaryMonitor.h"

#include <thread>
#include <chrono>

#if defined( __linux__ ) || defined ( __APPLE__ )
#define UNUSED(X) (void)(X)
#elif defined(_WIN32)
#define UNUSED(X) X;
#endif // __linux__

//////////////////////////////////////////////////////////////////////////

StatusBar::~StatusBar( void )
{
	m_Height  = 0;
	m_Width   = 0;
	m_Message = {};
	m_Text    = "";
	m_Active  = false;
} // ~StatusBar

//////////////////////////////////////////////////////////////////////////

// TODO: I'm not sure if we need this

void StatusBar::Init( void )
{
	m_Height = 24;
	m_Width = PrimaryMonitor::Instance().Width();

	SetText( "Ready" );
} // Init

  //////////////////////////////////////////////////////////////////////////

void StatusBar::SetColor( int r, int g, int b )
{
	m_Col_R = r;
	m_Col_G = g;
	m_Col_B = b;
} // SetColor

void StatusBar::SetColor( Color color )
{
	m_Col_R = 0;
	m_Col_G = 0;
	m_Col_B = 0;

	switch( color )
	{
		case Color::DEFAULT:
			m_Col_R = -1;
			m_Col_G = -1;
			m_Col_B = -1;
			break;
		case Color::BLACK:
			m_Col_R = 0;
			m_Col_G = 0;
			m_Col_B = 0;
			break;
		case Color::BLUE:
			m_Col_R = 70;
			m_Col_G = 129;
			m_Col_B = 224;
			break;
		case Color::ORANGE:
			m_Col_R = 202;
			m_Col_G = 81;
			m_Col_B = 0;
			break;
		case Color::RED:
			m_Col_R = 135;
			m_Col_G = 20;
			m_Col_B = 20;
			break;
		default:
			m_Col_R = -1;
			m_Col_G = -1;
			m_Col_B = -1;
			break;
	}

} // SetColor

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetText( std::string txt )
{
	m_Text = std::move( txt );

	m_Message = {};
	m_Message.Msg = m_Text;
} // Set Text

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetCurrentFileInfo( int column, int row, int length, int lines /*= 0 */ )
{

	m_TextEditInfo = "Col :  "
		+                    std::to_string ( column )
		+ "    Row :  "    + std::to_string ( row    )
		+ "    Length :  " + std::to_string ( length )
		+ "    Lines :  "  + std::to_string ( lines  );
} // SetCurrentFileInfo

//////////////////////////////////////////////////////////////////////////

void StatusBar::Show()
{
	m_Active = true;

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos( ImVec2( viewport->Pos.x, viewport->Pos.y + viewport->Size.y - static_cast< float >( m_Height ) ) );
	ImGui::SetNextWindowSize( ImVec2( viewport->Size.x, static_cast< float >( m_Height ) ) );
	ImGui::SetNextWindowViewport( viewport->ID );

	const ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoSavedSettings
		;

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 2, 2 ) );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0 );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0 );

	if( m_Col_R == -1 && m_Col_G == -1 && m_Col_B == -1 )
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( ImColor( 0xF0202020 ) ) );
	else
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( ImColor( m_Col_R, m_Col_G, m_Col_B ) ) );

	if( m_Message.Msg != "Ready" )
	{
		int64_t expiryInUnixTime = m_Message.Timestamp + m_Message.ExpiryTime;
		int64_t currentTime = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();

		if( currentTime == expiryInUnixTime )
		{
			m_Message = {};
			SetText( "Ready" );
		}
	}

	if( ImGui::Begin( "Status Bar", &m_Active, window_flags ) )
	{
		ImGui::Text( "%s", m_Message.Msg.c_str() );

		if( m_TextEditInfo != "" )
		{
			const char* txt = m_TextEditInfo.c_str();

			ImVec2 textSize = ImGui::CalcTextSize( txt );
			ImGui::SameLine( ImGui::GetWindowWidth() - 30 - textSize.x - textSize.y );
			ImGui::Text( "%s", txt );
		}

	}

	ImGui::PopStyleVar( 3 );

	ImGui::PopStyleColor( 1 );

	ImGui::End();

} // Show