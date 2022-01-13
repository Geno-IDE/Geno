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

#include "ProjectSettingsModal.h"

#include "Components/Project.h"
#include "Components/Workspace.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <array>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

enum Category
{
	CategoryGeneral,
	CategoryCompiler,
	CategoryLinker,
	NumCategories
};

//////////////////////////////////////////////////////////////////////////

static constexpr const char* StringifyCategory( Category Category )
{
	switch( Category )
	{
		case CategoryGeneral:  return "General";
		case CategoryCompiler: return "Compiler";
		case CategoryLinker:   return "Linker";
		default:               return nullptr;
	}

} // StringifyCategory

//////////////////////////////////////////////////////////////////////////

void ProjectSettingsModal::Show( std::string Project )
{
	if( Open() )
	{
		m_CurrentCategory = -1;
		m_EditedProject   = std::move( Project );
	}

} // Show

//////////////////////////////////////////////////////////////////////////

void ProjectSettingsModal::UpdateDerived( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		ImGui::TextUnformatted( "No active workspace" );
		return;
	}

	Project* pProject = pWorkspace->ProjectByName( m_EditedProject );
	if( !pProject )
	{
		ImGui::Text( "No project found by the name '%s'", m_EditedProject.c_str() );
		return;
	}

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
			switch( m_CurrentCategory )
			{
				case CategoryGeneral:
				{
					const std::array KindNames   = { "Application", "Static Library", "Dynamic Library" };
					int              CurrentItem = static_cast< int >( pProject->m_Kind ) - 1;

					ImGui::TextUnformatted( "Kind" );

					ImGui::SetNextItemWidth( -5.0f );
					if( ImGui::Combo( "##Kind", &CurrentItem, KindNames.data(), static_cast< int >( KindNames.size() ) ) )
					{
						pProject->m_Kind = static_cast< Project::Kind >( CurrentItem + 1 );
					}

				} break;

				case CategoryCompiler:
				{
					ImGui::TextUnformatted( "Include Directories" );

					for( size_t i = 0; i < pProject->m_LocalConfiguration.m_IncludeDirs.size(); ++i )
					{
						std::filesystem::path& rIncludeDir = pProject->m_LocalConfiguration.m_IncludeDirs[ i ];
						std::string            Buffer      = rIncludeDir.lexically_relative( pProject->m_Location ).string();
						const std::string      Label       = "##INCLUDEDIR_" + std::to_string( i );

						if( ImGui::InputText( Label.c_str(), &Buffer ) )
						{
							rIncludeDir = ( pProject->m_Location / Buffer ).lexically_normal();
						}
					}

					if( ImGui::SmallButton( "+##ADD_INCLUDE_DIR" ) )
					{
						pProject->m_LocalConfiguration.m_IncludeDirs.emplace_back();
					}

					ImGui::Separator();
					ImGui::TextUnformatted( "Preprocessor Defines" );

					for( size_t i = 0; i < pProject->m_LocalConfiguration.m_Defines.size(); ++i )
					{
						std::string&      rDefine = pProject->m_LocalConfiguration.m_Defines[ i ];
						const std::string Label   = "##DEFINE_" + std::to_string( i );

						ImGui::InputText( Label.c_str(), &rDefine );
					}

					if( ImGui::SmallButton( "+##ADD_DEFINE" ) )
					{
						pProject->m_LocalConfiguration.m_Defines.emplace_back();
					}

				} break;

				case CategoryLinker:
				{
					if( pProject->m_Kind == Project::Kind::StaticLibrary )
					{
						ImGui::TextUnformatted( "There are no linker settings for static libraries!" );
						break;
					}

					ImGui::TextUnformatted( "Library Directories" );

					for( size_t i = 0; i < pProject->m_LocalConfiguration.m_LibraryDirs.size(); ++i )
					{
						std::filesystem::path& rLibraryDir = pProject->m_LocalConfiguration.m_LibraryDirs[ i ];
						std::string            Buffer      = rLibraryDir.lexically_relative( pProject->m_Location ).string();
						const std::string      Label       = "##LIBRARYDIR_" + std::to_string( i );

						if( ImGui::InputText( Label.c_str(), &Buffer ) )
						{
							rLibraryDir = ( pProject->m_Location / Buffer ).lexically_normal();
						}
					}

					if( ImGui::SmallButton( "+##ADD_LIBRARY_DIR" ) )
					{
						pProject->m_LocalConfiguration.m_LibraryDirs.emplace_back();
					}

					ImGui::TextUnformatted( "Libraries" );

					for( size_t i = 0; i < pProject->m_LocalConfiguration.m_Libraries.size(); ++i )
					{
						std::string&      rLibrary = pProject->m_LocalConfiguration.m_Libraries[ i ];
						const std::string Label    = "##LIBRARY_" + std::to_string( i );

						ImGui::InputText( Label.c_str(), &rLibrary );
					}

					if( ImGui::SmallButton( "+##ADD_LIBRARY" ) )
					{
						pProject->m_LocalConfiguration.m_Libraries.emplace_back();
					}

				} break;
			}

		} ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();

	} ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Save & Close", ImVec2( 100, 0 ) ) )
	{
		pProject->Serialize();
		Close();
	}

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void ProjectSettingsModal::OnClose( void )
{
	m_CurrentCategory = -1;
	m_EditedProject.clear();

} // OnClose
