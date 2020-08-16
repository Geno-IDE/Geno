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

#include "NewItemModal.h"

#include "GUI/Modals/OpenFileModal.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void NewItemModal::Request( std::string_view title, void* user, Callback callback )
{
	if( Open() )
	{
		title_    = title;
		callback_ = callback;
		user_     = user;
	}
}

void NewItemModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1 , ImVec2( 0, -24 ) ) )
	{
		ImGui::TextUnformatted( "Name" );

		ImGui::SetNextItemWidth( -5.0f );
		ImGui::InputText( "##Name", &name_ );

		ImGui::TextUnformatted( "Location" );

		ImGui::SetNextItemWidth( -60.0f );
		ImGui::InputText( "##Location", &location_ );

		ImGui::SameLine();
		if( ImGui::Button( "Browse" ) )
		{
			OpenFileModal::Instance().RequestDirectory( title_ + " Location", this,
				[]( const std::filesystem::path& path, void* user )
				{
					NewItemModal* self = static_cast< NewItemModal* >( user );

					self->location_ = path.lexically_normal().string();
				}
			);
		}
	}
	ImGui::EndChild();

	if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
	{
		if( callback_ )
		{
			std::filesystem::path location_path = location_;

			callback_( name_, std::move( location_path ), user_ );
		}

		Close();
	}

	ImGui::SameLine();
	if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
	{
		Close();
	}
}
