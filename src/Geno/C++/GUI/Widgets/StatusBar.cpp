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

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetColor( int R, int G, int B )
{
	m_Col_R = R;
	m_Col_G = G;
	m_Col_B = B;

} // SetColor

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetColor( Color Color )
{
	m_Col_R = 0;
	m_Col_G = 0;
	m_Col_B = 0;

	switch( Color )
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

void StatusBar::SetText( std::string Text )
{
	m_Text        = std::move( Text );
	m_Message     = { };
	m_Message.Msg = m_Text;

} // SetText

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetCurrentFileInfo( int Column, int Row, int Length, int Lines )
{
	m_TextEditInfo = "Col :  "    + std::to_string( Column ) + "    " +
	                 "Row :  "    + std::to_string( Row )    + "    " +
	                 "Length :  " + std::to_string( Length ) + "    " +
	                 "Lines :  "  + std::to_string( Lines );

} // SetCurrentFileInfo

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetSearchResultInfo( std::string SearchTerm, int Results )
{
	if( SearchTerm.empty() )
	{
		m_TextEditSearchInfo = "";
		return;
	}

	m_TextEditSearchInfo = "Search: \"" + SearchTerm + "\"  |  Matches: " + std::to_string( Results );
} // SetSearchResultInfo

//////////////////////////////////////////////////////////////////////////

float StatusBar::GetHeight( void )
{
	return ImGui::GetFrameHeight();

} // GetHeight

//////////////////////////////////////////////////////////////////////////

void StatusBar::Show( void )
{
	const float Height = GetHeight();

	m_Active = true;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos( ImVec2( pViewport->Pos.x, pViewport->Pos.y + pViewport->Size.y - Height ) );
	ImGui::SetNextWindowSize( ImVec2( pViewport->Size.x, Height ) );
	ImGui::SetNextWindowViewport( pViewport->ID );

	const ImGuiWindowFlags WindowFlags = 0
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
		int64_t ExpiryInUnixTime = m_Message.Timestamp + m_Message.ExpiryTime;
		int64_t CurrentTime = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();

		if( CurrentTime == ExpiryInUnixTime )
		{
			m_Message = {};
			SetText( "Ready" );
		}
	}

	if( ImGui::Begin( "Status Bar", &m_Active, WindowFlags ) )
	{
		ImGui::Text( "%s", m_Message.Msg.c_str() );

		float Offset = 0.0f;

		if( !m_TextEditInfo.empty() )
		{
			const ImVec2 TextSize = ImGui::CalcTextSize( m_TextEditInfo.c_str() );
			Offset                = ImGui::GetWindowWidth() - 30 - TextSize.x - TextSize.y;

			ImGui::SameLine( Offset );
			ImGui::TextUnformatted( m_TextEditInfo.c_str() );
		}

		if( !m_TextEditSearchInfo.empty() )
		{
			const ImVec2 TextSize = ImGui::CalcTextSize( m_TextEditSearchInfo.c_str() );
			Offset -= TextSize.x + 30;

			ImGui::SameLine( Offset );
			ImGui::TextUnformatted( m_TextEditSearchInfo.c_str() );
		}
	}

	ImGui::PopStyleVar( 3 );
	ImGui::PopStyleColor( 1 );
	ImGui::End();

} // Show
