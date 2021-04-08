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

#include "SettingsModal.h"

#include "Compilers/ICompiler.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/MainWindow.h"
#include "Misc/Settings.h"

#include <array>
#include <iostream>

#include <Common/LocalAppData.h>
#include <GCL/Object.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "SettingsModal.h"

enum Category
{
	CategoryCompiler,
	CategoryTheme,
	NumCategories
};

//////////////////////////////////////////////////////////////////////////

static constexpr const char* StringifyCategory( Category Category )
{
	switch( Category )
	{
		case CategoryCompiler: return "Compiler";
		case CategoryTheme:    return "Theme";
		default:               return nullptr;
	}

} // StringifyCategory

//////////////////////////////////////////////////////////////////////////

void SettingsModal::Show( GCL::Object* pObject )
{
	if( !pObject->IsTable() )
	{
		std::cerr << "SettingsModal error: Trying to edit non-table object\n";
		return;
	}

	if( pObject->Empty() )
	{
		std::cerr << "SettingsModal error: Trying to edit empty table\n";
		return;
	}

	if( Open() )
	{
		m_CurrentCategory = -1;
		m_pEditedObject    = pObject;
	}

} // Show

//////////////////////////////////////////////////////////////////////////

void SettingsModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1, ImVec2( 0, -30 ) ) )
	{
		MainWindow::Instance().PushHorizontalLayout();

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_FrameBg ) );
		if( ImGui::BeginChild( 1, ImVec2( 120, 0 ) ) )
		{
			for( int i = 0; i < NumCategories; ++i )
			{
				Category Category = static_cast< ::Category >( i );

				if( ImGui::Selectable( StringifyCategory( Category ), m_CurrentCategory == Category ) )
				{
					m_CurrentCategory = Category;
				}
			}

		} ImGui::EndChild();
		ImGui::PopStyleColor();

		if( ImGui::BeginChild( 2 ) )
		{
			Settings& rSettings = Settings::Instance();

			switch( m_CurrentCategory )
			{
				case CategoryCompiler:
				{

				#if defined( _WIN32 )

					GCL::Object& rMinGWPath      = rSettings.m_Object[ "MinGW-Path" ];
					std::string  MinGWPathBuffer = rMinGWPath.IsString() ? rMinGWPath.String() : std::string();

					ImGui::TextUnformatted( "MinGW Path" );

					ImGui::SetNextItemWidth( -60.0f );
					if( ImGui::InputText( "##MinGW Path", &MinGWPathBuffer ) )
					{
						rMinGWPath.SetString( MinGWPathBuffer );
					}

					ImGui::SameLine();
					ImGui::SetNextItemWidth( 50.0f );
					if( ImGui::Button( "Browse" ) )
					{
						OpenFileModal::Instance().RequestDirectory( "Locate MinGW Directory", this,
							[]( const std::filesystem::path& path, void* /*user*/ )
							{
								Settings& rSettings = Settings::Instance();

								rSettings.m_Object[ "MinGW-Path" ] = path.string();
							}
						);
					}

				#endif // _WIN32

				} break;

				case CategoryTheme:
				{
					const std::array ThemeNames   = { "Classic", "Light", "Dark" };
					GCL::Object&     rTheme       = rSettings.m_Object[ "Theme" ];
					auto             CurrentTheme = rTheme.IsString() ? std::find( ThemeNames.begin(), ThemeNames.end(), rTheme.String() ) : ThemeNames.end();
					int              CurrentItem  = ( CurrentTheme == ThemeNames.end() ) ? -1 : static_cast< int >( std::distance( ThemeNames.begin(), CurrentTheme ) );

					ImGui::TextUnformatted( "Theme" );

					ImGui::SetNextItemWidth( -5.0f );
					if( ImGui::Combo( "##Theme", &CurrentItem, ThemeNames.data(), static_cast< int >( ThemeNames.size() ) ) )
					{
						rTheme = ThemeNames[ CurrentItem ];
						rSettings.UpdateTheme();
					}

				} break;
			}

		} ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();

	} ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Close", ImVec2( 80, 0 ) ) )
	{
		Close();
	}

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void SettingsModal::OnClose( void )
{
	m_CurrentCategory = 0;
	m_pEditedObject   = nullptr;

} // OnClose
