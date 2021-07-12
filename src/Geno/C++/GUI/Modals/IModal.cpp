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

#include "IModal.h"

#include "Application.h"
#include "Auxiliary/ImGuiAux.h"

//////////////////////////////////////////////////////////////////////////

void IModal::Update( void )
{
	const std::string PopupID = Title() + "##" + this->PopupID();

	if( m_Open && !ImGui::IsPopupOpen( PopupID.c_str() ) )
	{
		ImGui::OpenPopup( PopupID.c_str() );
	}

	ImGui::SetNextWindowSize( ImVec2( 350, 196 ), ImGuiCond_FirstUseEver );

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
	bool IsOpen = ImGui::BeginPopupModal( PopupID.c_str(), NULL, ImGuiWindowFlags_NoTitleBar );
	ImGui::PopStyleVar(2);

	if( IsOpen )
	{
		ImGuiAux::TextCentered(Title().c_str());

		if(ImGui::BeginChild("Content", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar))
		{
			UpdateDerived();

			if( IModal* next = Application::Instance().NextModal( this ) )
				next->Update();
			
		} ImGui::EndChild();

		ImGui::EndPopup();

		if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
			Close();
	}

} // Update

//////////////////////////////////////////////////////////////////////////

void IModal::Close( void )
{
	m_Open = false;

	ImGui::CloseCurrentPopup();

	Application::Instance().PopModal();

	OnClose();

} // Close

//////////////////////////////////////////////////////////////////////////

bool IModal::Open( void )
{
	// Make sure this popup is not already opened
	if( m_Open )
		return false;

	Application::Instance().PushModal( this );

	m_Open 	  = true;

	return true;

} // Open
