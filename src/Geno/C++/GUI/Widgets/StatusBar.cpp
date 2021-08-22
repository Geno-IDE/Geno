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

//////////////////////////////////////////////////////////////////////////

StatusBar::~StatusBar( void )
{

} // ~StatusBar

//////////////////////////////////////////////////////////////////////////

void StatusBar::Init( void )
{
	m_Height = 24;
	m_Width = PrimaryMonitor::Instance().Width();
} // Init

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetColor( float r, float g, float b )
{
	m_Color = { r,g,b, 0.0f };
}

void StatusBar::SetColor( ImVec4 color )
{
	m_Color = color;
}

//////////////////////////////////////////////////////////////////////////

void StatusBar::SetText( std::string txt )
{
	m_Text = txt;
}

void StatusBar::SetText( const char* txt )
{
	m_Text = txt;
}

//////////////////////////////////////////////////////////////////////////

void StatusBar::Show( bool* pOpen )
{
	m_Active = pOpen;

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

	ImGui::PushStyleColor( ImGuiCol_WindowBg, m_Color );


	if( ImGui::Begin( "Status Bar", &m_Active, window_flags ) )
	{
		ImGui::Text( m_Text.c_str() );
	}

	ImGui::PopStyleVar( 3 );
	ImGui::PopStyleColor( 1 );
	ImGui::End();

} // Show
