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

enum RequestType
{
	None = 0,
	RequestTypePath,
	RequestTypeString,
};

void NewItemModal::RequestPath( std::string_view title, void* user, PathCallback callback )
{
	if( Open() )
	{
		title_        = title;
		callback_     = callback;
		user_         = user;
		request_type_ = RequestTypePath;
	}
}

void NewItemModal::RequestString( std::string_view title, void* user, StringCallback callback )
{
	if( Open() )
	{
		title_        = title;
		callback_     = callback;
		user_         = user;
		request_type_ = RequestTypeString;
	}
}

void NewItemModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1 , ImVec2( 0, -24 ) ) )
	{
		ImGui::TextUnformatted( "Name" );

		switch( request_type_ )
		{
			case RequestTypePath:   { UpdateItem();   } break;
			case RequestTypeString: { UpdateString(); } break;
		}
	}
	ImGui::EndChild();

	if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
	{
		if( callback_ )
		{
			switch( request_type_ )
			{
				case RequestTypePath:   { static_cast< PathCallback   >( callback_ )( name_, location_, user_ ); } break;
				case RequestTypeString: { static_cast< StringCallback >( callback_ )( name_, user_ );            } break;
				default:                { GENO_ASSERT( false ); /* Request type was corrupted */                 } break;
			}
		}

		Close();
	}

	ImGui::SameLine();
	if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
	{
		Close();
	}
}

void NewItemModal::OnClose( void )
{
	title_.clear();
	name_.clear();
	location_.clear();

	callback_     = nullptr;
	user_         = nullptr;
	request_type_ = -1;
}

void NewItemModal::UpdateItem( void )
{
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

void NewItemModal::UpdateString( void )
{
	ImGui::SetNextItemWidth( -5.0f );
	ImGui::InputText( "##Name", &name_ );
}
