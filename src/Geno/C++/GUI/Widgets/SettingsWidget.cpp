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

#include "SettingsWidget.h"

#include "Common/LocalAppData.h"
#include "Compilers/Compiler.h"
#include "GUI/MainWindow.h"

#include <array>
#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>

SettingsWidget::SettingsWidget( void )
{
#if defined( _WIN32 )

	if( std::ifstream ifs( LocalAppData::Instance() / "mingw-path.txt" ); ifs.good() )
	{
		ifs >> mingw_path_;

		Compiler::Instance().SetPath( mingw_path_.native() );
	}

#elif defined( __linux__ ) // _WIN32
	
	if( std::ifstream ifs( LocalAppData::Instance() / "llvm-path.txt" ); ifs.good() )
	{
		ifs >> llvm_path_;

		Compiler::Instance().SetPath( llvm_path_.native() );
	}

#endif // __linux__

	if( std::ifstream ifs( LocalAppData::Instance() / "theme.txt" ); ifs.good() )
	{
		// #TODO: This should be a string. Otherwise it's going to load the wrong theme if we add a new one with an index lower than the saved theme.
		ifs >> current_theme_;

		UpdateTheme();
	}
}

void SettingsWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Settings", p_open ) )
	{
		constexpr float       list_width   = 120.f;
		constexpr const char* list_items[] = { "Compiler", "Theme" };

		MainWindow::Instance().PushHorizontalLayout();

		if( ImGui::BeginChild( 1, ImVec2( list_width, 0.f ) ) )
		{
			for( size_t i = 0; i < std::size( list_items ); ++i )
			{
				if( ImGui::Selectable( list_items[ i ], current_panel_item_ == ( int )i ) )
				{
					current_panel_item_ = ( int )i;
				}
			}
		}
		ImGui::EndChild();

		if( ImGui::BeginChild( 2 ) )
		{
			switch( current_panel_item_ )
			{
				case 0:
				{

			#if defined( _WIN32 )

					char mingw_path_buf[ FILENAME_MAX + 1 ] = { };

					for( size_t i = 0; i < mingw_path_.native().size(); ++i )
					{
						mingw_path_buf[ i ] = ( char )mingw_path_.native().at( i );
					}

					if( ImGui::InputText( "MinGW Path", &mingw_path_buf[ 0 ], std::size( mingw_path_buf ) ) )
					{
						mingw_path_ = mingw_path_buf;

						Compiler::Instance().SetPath( mingw_path_.native() );

						std::ofstream ofs( LocalAppData::Instance() / "mingw-path.txt", std::ios::binary | std::ios::trunc );
						ofs << mingw_path_;
					}

			#elif defined( __linux__ ) // _WIN32

					char llvm_path_buf[ FILENAME_MAX + 1 ] = { };

					for( size_t i = 0; i < llvm_path_.native().size(); ++i )
					{
						llvm_path_buf[ i ] = ( char )llvm_path_.native().at( i );
					}

					if( ImGui::InputText( "LLVM Path", &llvm_path_buf[ 0 ], std::size( llvm_path_buf ) ) )
					{
						llvm_path_ = llvm_path_buf;

						Compiler::Instance().SetPath( llvm_path_.native() );

						std::ofstream ofs( LocalAppData::Instance() / "llvm-path.txt", std::ios::binary | std::ios::trunc );
						ofs << llvm_path_;
					}

			#endif // __linux__

				} break;

				case 1:
				{
					if( ImGui::Combo( "Theme", &current_theme_, "Classic\0Light\0Dark\0" ) )
					{
						UpdateTheme();

						std::ofstream ofs( LocalAppData::Instance() / "theme.txt", std::ios::binary | std::ios::trunc );
						ofs << current_theme_;
					}

				} break;
			}
		}
		ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();
	}
	ImGui::End();
}

void SettingsWidget::UpdateTheme( void )
{
	switch( current_theme_ )
	{
		case 0: { ImGui::StyleColorsClassic(); } break;
		case 1: { ImGui::StyleColorsLight();   } break;
		case 2: { ImGui::StyleColorsDark();    } break;
	}
}
