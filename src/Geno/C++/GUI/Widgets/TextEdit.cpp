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

#include "TextEdit.h"

#include "Common/Drop.h"
#include "Common/LocalAppData.h"
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <fstream>
#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

const char* WINDOW_NAME = "Text Edit";

//////////////////////////////////////////////////////////////////////////

TextEdit::TextEdit( void )
{
	// Create tab bar
	{
		ImGuiContext* pContext = ImGui::GetCurrentContext();
		ImGuiID       ID       = ImHashStr( "TextEditTabBar" ); // ImGui::GetID( "TextEditTabBar" );
		m_pTabBar              = pContext->TabBars.GetOrAddByKey( ID );
		m_pTabBar->ID          = ID;
	}

} // TextEdit

//////////////////////////////////////////////////////////////////////////

void TextEdit::Show( bool* pOpen )
{
	ImGuiStyle& rStyle          = ImGui::GetStyle();
	ImVec4      BackgroundColor = rStyle.Colors[ ImGuiCol_WindowBg ];

	// Use a brighter background color if the widget is being drag-hovered
	if( const Drop* pDrop = MainWindow::Instance().GetDraggedDrop() )
	{
		const float DragX = static_cast< float >( MainWindow::Instance().GetDragPosX() );
		const float DragY = static_cast< float >( MainWindow::Instance().GetDragPosY() );

		if( ImGuiWindow* pWindow = ImGui::FindWindowByName( WINDOW_NAME ) )
		{
			if( pWindow->Rect().Contains( ImVec2( DragX, DragY ) ) )
			{
				BackgroundColor = BackgroundColor + ImVec4( 0.1f, 0.1f, 0.1f, 0.1f );

				switch( pDrop->GetType() )
				{
					case Drop::TypeIndex::Bitmap:
					{
						const Drop::Bitmap& rBitmap = pDrop->GetBitmap();

						m_DraggedBitmapTexture.SetPixels( GL_RGBA8, rBitmap.width, rBitmap.height, GL_BGRA, rBitmap.data.get() );

						// Adjust image size and ensure that no dimension is bigger than 200px
						constexpr float ImageMaxSize = 200.0f;
						ImVec2          ImageSize;
						if( rBitmap.width > rBitmap.height ) ImageSize = ImVec2( ImageMaxSize, ImageMaxSize * ( rBitmap.height / static_cast< float >( rBitmap.width ) ) );
						else                                 ImageSize = ImVec2( ImageMaxSize * ( rBitmap.width / static_cast< float >( rBitmap.height ) ), ImageMaxSize );

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Image( m_DraggedBitmapTexture.GetID(), ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Text:
					{
						const Drop::Text& rText = pDrop->GetText();
						
						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Text( "%ws", rText.c_str() );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Paths:
					{
						const Drop::Paths& rPaths = pDrop->GetPaths();
						
						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();

						for( const std::filesystem::path& rPath : rPaths )
							ImGui::BulletText( "%ws", rPath.c_str() );

						ImGui::EndTooltip();

					} break;
				}
			}
		}
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, BackgroundColor );
	ImGui::SetNextWindowSize( ImVec2( 350, 196 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( WINDOW_NAME, pOpen ) )
	{
		const int           TabBarFlags  = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_IsFocused;
		const ImGuiContext* pContext     = ImGui::GetCurrentContext();
		const ImGuiWindow*  pWindow      = ImGui::GetCurrentWindow();
		const ImRect        TabBarBounds = ImRect( pWindow->DC.CursorPos.x, pWindow->DC.CursorPos.y, pWindow->WorkRect.Max.x, pWindow->DC.CursorPos.y + pContext->FontSize + pContext->Style.FramePadding.y * 2 );

		if( !m_Files.empty() && ImGui::BeginTabBarEx( m_pTabBar, TabBarBounds, TabBarFlags, nullptr ) )
		{
			for( File& rFile : m_Files )
			{
				const std::string FileString = rFile.Path.filename().string();

				if( ImGui::BeginTabItem( FileString.c_str(), &rFile.Open ) )
				{
					const int InputTextFlags = ImGuiInputTextFlags_AllowTabInput;

					if( ImGui::InputTextMultiline( "##TextEditor", &rFile.Text, ImVec2( -0.01f, -0.01f ), InputTextFlags ) )
					{
						std::ofstream ofs( rFile.Path, std::ios::binary | std::ios::trunc );
						ofs << rFile.Text;
					}

					ImGui::EndTabItem();
				}
			}

			// Clear closed files from list
			for( auto It = m_Files.begin(); It != m_Files.end(); )
			{
				if( It->Open ) It++;
				else           It = m_Files.erase( It );
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();

} // Show

//////////////////////////////////////////////////////////////////////////

void TextEdit::AddFile( const std::filesystem::path& rPath )
{
	if( !std::filesystem::exists( rPath ) )
	{
		std::cerr << "Failed to add '" << rPath << "' to text-edit. File does not exist.\n";
		return;
	}

//////////////////////////////////////////////////////////////////////////

	std::ifstream     InputFileStream( rPath, std::ios::binary );
	const std::string Text( ( std::istreambuf_iterator< char >( InputFileStream ) ), std::istreambuf_iterator< char >() );

	for( size_t i = 0; i < m_Files.size(); ++i )
	{
		File& rFile = m_Files[ i ];

		// Do not need to add file to vector if it already exists
		if( rFile.Path == rPath )
		{
			// Select the tab that corresponds to the open file
			m_pTabBar->NextSelectedTabId = m_pTabBar->Tabs[ static_cast< int >( i ) ].ID;

			// Update text in case file changed externally
			rFile.Text = Text;
			return;
		}
	}

	File File;
	File.Path = rPath;
	File.Text = Text;

	m_Files.emplace_back( std::move( File ) );

} // AddFile

//////////////////////////////////////////////////////////////////////////

void TextEdit::OnDragDrop( const Drop& rDrop, int X, int Y )
{
	ImGuiWindow* pWindow = ImGui::FindWindowByName( WINDOW_NAME );

	if( pWindow && pWindow->Rect().Contains( ImVec2( static_cast< float >( X ), static_cast< float >( Y ) ) ) )
	{
		switch( rDrop.GetType() )
		{
			case Drop::TypeIndex::Paths:
			{
				const Drop::Paths& rPaths = rDrop.GetPaths();

				for( const std::filesystem::path& rPath : rPaths )
					AddFile( rPath );

			} break;
		}
	}

} // OnDragDrop
