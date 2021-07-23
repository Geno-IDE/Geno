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

#include "MessageModal.h"

#include "Auxiliary/ImGuiAux.h"

#include <imgui.h>

void MessageModal::ShowMessage( std::string Message, const char* pButtonLabel, Callback Callback )
{
	if( Open() )
	{
		m_Message      = Message;
		m_pButtonLabel = pButtonLabel;
		m_Callback     = Callback;

		ImVec2 Offset = ImGui::CalcTextSize( m_Message.c_str() );

		m_MinSize = ImVec2( 350.0f, ( Offset.y + 70.0f ) * 2.0f );
		m_MaxSize = m_MinSize;
	}
} // ShowMessage

//////////////////////////////////////////////////////////////////////////

void MessageModal::UpdateDerived( void )
{
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 5.0f, 5.0f ) );
	if( ImGui::BeginChild( 1, ImVec2( 0, -44 ) ) )
	{
		ImGui::TextWrapped( "%s", m_Message.c_str() );
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	m_ButtonData.Size = ImVec2( 70, 30 );

	if( ImGuiAux::Button( m_pButtonLabel, m_ButtonData ) )
	{
		m_Callback();
		Close();
	}

	ImGui::SameLine();

	if( ImGuiAux::Button( "Cancel", m_ButtonData ) )
	{
		Close();
	}

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void MessageModal::OnClose( void )
{
	m_Message.clear();
} // OnClose
