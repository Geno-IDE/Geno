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

#include "GenoDiscordSettingsModal.h"
#include "Discord/DiscordRPC.h"

std::string GenoDiscordSettingsModal::PopupID( void )
{
	return "EXT_GENO_DISCORD_MODAL";
} // PopupID

std::string GenoDiscordSettingsModal::Title( void )
{
	return "Geno Discord Settings";
} // Title

void GenoDiscordSettingsModal::UpdateDerived( void )
{
	ImGui::Text( "Show File Name" );      ImGui::SameLine(); ImGui::Checkbox( "##gd-ext-file",  &GenoDiscord::Instance().m_Settings.ShowFilename );
	ImGui::Text( "Show Workspace Name" ); ImGui::SameLine(); ImGui::Checkbox( "##gd-ext-wks",   &GenoDiscord::Instance().m_Settings.ShowWrksName );
	ImGui::Text( "Show Time" );           ImGui::SameLine(); ImGui::Checkbox( "##gd-ext-time",  &GenoDiscord::Instance().m_Settings.ShowTime );
	ImGui::Text( "Show" );                ImGui::SameLine(); ImGui::Checkbox( "##gd-ext-show",  &GenoDiscord::Instance().m_Settings.Show );

	if( ImGui::Button( "Close" ) )
		Close();

} // UpdateDerived

void GenoDiscordSettingsModal::Show()
{
} // Show
