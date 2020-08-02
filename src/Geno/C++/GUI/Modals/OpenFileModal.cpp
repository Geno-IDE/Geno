/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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

#include "OpenFileModal.h"

#include <Common/LocalAppData.h>

#include <imgui.h>

static bool                  open_modal      = false;
static OpenFileModalCallback global_callback = nullptr;
static void*                 global_user     = nullptr;

void OpenOpenFileModal( void* user, OpenFileModalCallback callback )
{
	open_modal      = true;
	global_user     = user;
	global_callback = callback;
}

void ShowOpenFileModal( void )
{
	if( open_modal )
		ImGui::OpenPopup( "OpenFile" );

	if( ImGui::BeginPopupModal( "OpenFile" ) )
	{
		std::filesystem::path path = LocalAppData::Instance() / "MyWorkspace.gwks";

		if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
		{
			if( global_callback )
				global_callback( path, global_user );

			global_callback = nullptr;
			global_user     = nullptr;

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
		{
			global_callback = nullptr;
			global_user     = nullptr;

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	open_modal = false;
}
