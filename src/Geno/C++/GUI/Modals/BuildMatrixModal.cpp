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

#include "BuildMatrixModal.h"

#include "Auxiliary/ImGuiAux.h"
#include "Auxiliary/STBAux.h"
#include "Compilers/CompilerGCC.h"
#include "Compilers/CompilerMSVC.h"
#include "Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

constexpr float COLUMN_WIDTH = 128.0f;

//////////////////////////////////////////////////////////////////////////

BuildMatrixModal::BuildMatrixModal( void )
	: m_TextureColumnMenuIcon( STBAux::LoadImageTexture( "Icons/ColumnMenu.png" ) )
	, m_TextureNewColumn     ( STBAux::LoadImageTexture( "Icons/NewColumn.png" ) )
{

} // BuildMatrixModal

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::Show( void )
{
	Open();

} // Show

//////////////////////////////////////////////////////////////////////////

std::string BuildMatrixModal::PopupID( void )
{
	return "BUILD_MATRIX_MODAL";

} // PopupID

//////////////////////////////////////////////////////////////////////////

std::string BuildMatrixModal::Title( void )
{
	return "Configuring Build Matrix";

} // Title

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::UpdateDerived( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		ImGui::TextUnformatted( "No workspace open" );
		return;
	}

	const bool ShowSidebar = m_SelectedColumnIndex >= 0 && m_SelectedConfigurationIndex >= 0;

	if( ImGuiAux::BeginChildHorizontal( ImGui::GetID( "Columns" ), ImVec2( ShowSidebar ? -200.0f : 0.0f, -20.0f ), false, ImGuiWindowFlags_HorizontalScrollbar ) )
	{
		DrawColumns();

		// Draw ghost column with a button that lets the user create a new column
		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.15f, 0.15f, 0.15f, 0.4f ) );
		if( ImGui::BeginChild( "GhostColumn", ImVec2( COLUMN_WIDTH, 0 ) ) )
		{
			const ImVec2 ButtonSize( COLUMN_WIDTH * 0.5f, COLUMN_WIDTH * 0.5f );

			ImGui::SetCursorPos( ( ImGui::GetWindowSize() - ButtonSize ) * 0.5f );

			const ImRect Rect( ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ButtonSize );
			bool         Hovering;
			bool         Held;

			if( ImGui::ButtonBehavior( Rect, ImGui::GetID( "ImageButton" ), &Hovering, &Held ) )
			{
				BuildMatrix::Column NewColumn;
				NewColumn.Name = "New Column";
				pWorkspace->m_BuildMatrix.m_Columns.emplace_back( std::move( NewColumn ) );
			}

			ImGui::Image( m_TextureNewColumn.GetID(), ButtonSize, ImVec2( Held ? 0.5f : 0.0f, 0.0f ), ImVec2( Held ? 1.0f : 0.5f, 1.0f ), Hovering ? ImVec4( 1, 1, 1, 1 ) : ImVec4( 0.8f, 0.8f, 0.8f, 1 ) );

		} ImGui::EndChild();
		ImGui::PopStyleColor();

	} ImGui::EndChild();

	if( ShowSidebar )
	{
		ImGui::SameLine();
		DrawSidebar();
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.3f, 0.3f, 0.3f, 0.4f ) );
	if( ImGui::BeginChild( ImGui::GetID( "BottomBar" ) ) )
	{
		if( ImGui::Button( "Save & Close" ) )
		{
			pWorkspace->Serialize();
			Close();
		}

	} ImGui::EndChild();
	ImGui::PopStyleColor();

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::DrawColumns( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
		return;

	BuildMatrix::ColumnVector& rColumns      = pWorkspace->m_BuildMatrix.m_Columns;
	ImGuiID                    ID            = ImGui::GetID( &rColumns );
	intptr_t                   IndexToRemove = -1;

	for( size_t i = 0; i < pWorkspace->m_BuildMatrix.m_Columns.size(); ++i )
	{
		BuildMatrix::Column& rColumn = rColumns[ i ];

		if( ImGui::BeginChild( ID++, ImVec2( COLUMN_WIDTH, 0 ), false, ImGuiWindowFlags_NoScrollbar ) )
		{
			if( ImGui::BeginChild( ID++, ImVec2( 0, 20 ) ) )
			{
				if( static_cast< ptrdiff_t >( i ) == m_ColumnNameEditedIndex )
				{
					const ImVec2 TextSize = ImGui::CalcTextSize( m_ColumnNameEditText.c_str() );

					ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
					ImGui::PushStyleColor( ImGuiCol_FrameBg, ImGui::GetStyleColorVec4( ImGuiCol_ChildBg ) );

					ImGui::SetCursorPosX( ( ImGui::GetWindowWidth() - TextSize.x ) * 0.5f );
					ImGui::SetNextItemWidth( COLUMN_WIDTH );
					ImGui::SetKeyboardFocusHere();
					if( ImGui::InputText( "##COLUMN_NAME_EDIT", &m_ColumnNameEditText, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue ) )
					{
						if( !m_ColumnNameEditText.empty() )
							rColumn.Name = m_ColumnNameEditText;

						m_ColumnNameEditedIndex = -1;
					}

					ImGui::PopStyleColor();
					ImGui::PopStyleVar();
				}
				else
				{
					ImGuiAux::TextCentered( rColumn.Name.c_str() );
				}

				const ImVec2 ImageSize( ImGui::GetFontSize(), ImGui::GetFontSize() );
				ImGui::SameLine( COLUMN_WIDTH - 12.0f );
				ImGui::Image( m_TextureColumnMenuIcon.GetID(), ImageSize );

				if( ImGui::IsItemHovered() )
				{
					ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );

					if( ImGui::IsItemClicked() )
						ImGui::OpenPopup( "Menu" );
				}

				if( ImGui::BeginPopup( "Menu", ImGuiPopupFlags_MouseButtonLeft ) )
				{
					if( ImGui::MenuItem( "Rename" ) )
					{
						m_ColumnNameEditedIndex = static_cast< ptrdiff_t >( i );
						m_ColumnNameEditText    = rColumn.Name;
						ImGui::CloseCurrentPopup();
					}

					if( ImGui::MenuItem( "Add Configuration" ) )
					{
						rColumn.Configurations.emplace_back( "New Configuration", Configuration() );

						ImGui::CloseCurrentPopup();
					}

					if( rColumns.size() > 1 )
					{
						if( i > 0 && ImGui::MenuItem( "Move Left" ) )
						{
							std::swap( rColumns[ i - 1 ], rColumns[ i ] );
							ImGui::CloseCurrentPopup();
						}

						if( i < ( rColumns.size() - 1 ) && ImGui::MenuItem( "Move Right" ) )
						{
							std::swap( rColumns[ i ], rColumns[ i + 1 ] );
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::Separator();

					if( ImGui::MenuItem( "Remove" ) )
					{
						IndexToRemove = static_cast< intptr_t >( i );
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::Separator();

			} ImGui::EndChild();

			if( ImGui::BeginChild( ID++ ) )
			{
				for( size_t j = 0; j < rColumn.Configurations.size(); ++j )
				{
					auto&        rConfiguration = rColumn.Configurations[ j ];
					ImGuiWindow* pWindow        = ImGui::GetCurrentWindow();
					const ImVec2 TextSize       = ImGui::CalcTextSize( rConfiguration.first.c_str() );
					const ImVec2 CursorPos      = pWindow->DC.CursorPos;
					const bool   Selected       = static_cast< ptrdiff_t >( i ) == m_SelectedColumnIndex && static_cast< ptrdiff_t >( j ) == m_SelectedConfigurationIndex;

					ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_ChildBg ) + ( Selected ? ImVec4( 0.5f, 0.5f, 0.5f, 0.5f ) : ImVec4( 0, 0, 0, 0 ) ) );

					if( ImGui::BeginChild( ID++, ImVec2( 0, TextSize.y ) ) )
					{
						if( ImGui::IsWindowHovered() )
						{
							ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
							ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( 0.0f,                                   0.0f ), 0xFFFFFFFF, ImGuiDir_Right );
							ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( pWindow->Size.x - ImGui::GetFontSize(), 0.0f ), 0xFFFFFFFF, ImGuiDir_Left );

							if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
							{
								m_SelectedColumnIndex        = i;
								m_SelectedConfigurationIndex = j;
								m_NameEditText               = rConfiguration.first;
							}
						}

						ImGui::SetCursorPosX( ( pWindow->Size.x - TextSize.x ) * 0.5f );
						ImGui::Text( rConfiguration.first.c_str() );

					} ImGui::EndChild();

					ImGui::PopStyleColor();
				}

			} ImGui::EndChild();

		} ImGui::EndChild();
		ImGui::Separator();
	}

	// Remove column that was removed by the user
	if( IndexToRemove >= 0 )
	{
		if( m_SelectedColumnIndex == IndexToRemove )
		{
			m_SelectedColumnIndex        = -1;
			m_SelectedConfigurationIndex = -1;
		}

		rColumns.erase( rColumns.begin() + IndexToRemove );
	}

} // DrawColumns

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::DrawSidebar( void )
{
	if( m_SelectedColumnIndex == -1 || m_SelectedConfigurationIndex == -1 )
		return;

	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
		return;

	const ImGuiID ID = ImGui::GetID( "Sidebar" );

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10, 10 ) );
	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.2f, 0.2f, 0.4f ) );

	if( ImGui::BeginChild( ID, ImVec2( 200, -20 ), true ) )
	{
		BuildMatrix::Column&             rColumn        = pWorkspace->m_BuildMatrix.m_Columns[ m_SelectedColumnIndex ];
		BuildMatrix::NamedConfiguration& rConfiguration = rColumn.Configurations[ m_SelectedConfigurationIndex ];

		// Name
		{
			bool NameIsValid = !m_NameEditText.empty();
			{
				auto      ConflictingConfiguration      = std::find_if( rColumn.Configurations.begin(), rColumn.Configurations.end(), [ this ]( const BuildMatrix::NamedConfiguration& rPair ) { return rPair.first == m_NameEditText; } );
				ptrdiff_t ConflictingConfigurationIndex = std::distance( rColumn.Configurations.begin(), ConflictingConfiguration );

				if( ConflictingConfiguration != rColumn.Configurations.end() && ConflictingConfigurationIndex != m_SelectedConfigurationIndex )
					NameIsValid = false;
			}

			ImGui::TextUnformatted( "Name" );

			ImVec4 EditBackgroundColor = ImGui::GetStyleColorVec4( ImGuiCol_FrameBg );
			if( !NameIsValid ) EditBackgroundColor.x += 0.5f;
			ImGui::PushStyleColor( ImGuiCol_FrameBg, EditBackgroundColor );

			// #TODO: Name could technically be changed the same frame as the user presses Enter, which will allow for invalid names to pass through
			if( ImGui::InputText( "##NAME", &m_NameEditText, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				// Name could technically have changed the same frame as the user presses the Enter key
				auto ConflictingConfiguration = std::find_if( rColumn.Configurations.begin(), rColumn.Configurations.end(), [ this ]( const BuildMatrix::NamedConfiguration& rPair ) { return rPair.first == m_NameEditText; } );
				NameIsValid = !m_NameEditText.empty() && ConflictingConfiguration == rColumn.Configurations.end();

				if( NameIsValid )
				{
					rConfiguration.first = m_NameEditText;
				}
				else
				{
					// Reset name edit text to signify that it wasn't changed successfully
					m_NameEditText = rConfiguration.first;
				}
			}

			ImGui::PopStyleColor();
		}

		ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 10.0f );

		// Compiler
		{
			const char* pCompilerNames[] = { "None", "MSVC", "GCC" };
			int         Index            = rConfiguration.second.m_Compiler ? static_cast< int >( std::distance( std::begin( pCompilerNames ), std::find_if( std::begin( pCompilerNames ), std::end( pCompilerNames ), [ &rConfiguration ]( const char* pName ) { return pName == rConfiguration.second.m_Compiler->GetName(); } ) ) ) : 0;

			ImGui::TextUnformatted( "Compiler" );

			if( ImGui::Combo( "##COMPILER", &Index, pCompilerNames, static_cast< int >( std::size( pCompilerNames ) ) ) )
			{
				switch( Index )
				{
					case 0: { rConfiguration.second.m_Compiler.reset();                              } break;
					case 1: { rConfiguration.second.m_Compiler = std::make_shared< CompilerMSVC >(); } break;
					case 2: { rConfiguration.second.m_Compiler = std::make_shared< CompilerGCC  >(); } break;
				}
			}
		}

		// Delete button
		{
			ImGui::SetCursorPosY( ImGui::GetWindowHeight() - 24 );
			ImGui::SetCursorPosX( ( 200 - 60 ) / 2 );

			if( ImGui::Button( "Delete##DELETE_CONFIGURATION", ImVec2( 60, 20 ) ) )
			{
				rColumn.Configurations.erase( rColumn.Configurations.begin() + m_SelectedConfigurationIndex );

				m_SelectedColumnIndex        = -1;
				m_SelectedConfigurationIndex = -1;
			}
		}

	} ImGui::EndChild();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

} // DrawSidebar
