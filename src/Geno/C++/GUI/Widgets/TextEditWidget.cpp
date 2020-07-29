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

#include "TextEditWidget.h"

#include "Core/LocalAppData.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"

#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>

TextEditWidget::TextEditWidget( void )
{
	if( std::ifstream ifs( LocalAppData::Instance() / L"build.cpp" ); ifs.is_open() )
	{
		text_.assign( ( std::istreambuf_iterator< char >( ifs ) ), std::istreambuf_iterator< char >() );
	}
	else
	{
		text_ = "#include <iostream>\n\nint main(int argc, char* argv[])\n{\n\tstd::cout << \"Hello, world!\\n\";\n\treturn 0;\n}\n";
	}
}

void TextEditWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Text Edit", p_open ) )
	{
		const int tab_bar_flags = ( 0
			| ImGuiTabBarFlags_Reorderable
			| ImGuiTabBarFlags_FittingPolicyScroll
		);

		if( ImGui::BeginTabBar( "TextEditTabBar", tab_bar_flags ) )
		{
			if( ImGui::BeginTabItem( "build.cpp" ) )
			{
				const int input_text_flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;

				if( ImGui::InputTextMultiline( "##TextEditor", &text_[ 0 ], text_.size() + 1, ImVec2( -0.01f, -0.01f ), input_text_flags, InputTextCB, this ) )
				{
					std::ofstream ofs( LocalAppData::Instance() / "build.cpp", std::ios::binary | std::ios::trunc );
					ofs << text_;
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

int TextEditWidget::InputTextCB( ImGuiInputTextCallbackData* data )
{
	TextEditWidget* self = ( TextEditWidget* )data->UserData;

	if( data->EventFlag == ImGuiInputTextFlags_CallbackResize )
	{
		self->text_.resize( data->BufTextLen );
		data->Buf = &self->text_[ 0 ];
	}

	return 0;
}
