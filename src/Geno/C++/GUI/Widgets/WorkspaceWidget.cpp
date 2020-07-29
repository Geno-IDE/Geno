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

#include "WorkspaceWidget.h"

#include <imgui.h>

void WorkspaceWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Workspace", p_open ) )
	{
		if( ImGui::TreeNode( "Workspace" ) )
		{
			if( ImGui::TreeNode( "MyApp" ) )
			{
				if( ImGui::TreeNode( "src" ) )
				{
					ImGui::Selectable( "main.cpp" );
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			if( ImGui::TreeNode( "MyLibrary" ) )
			{
				if( ImGui::TreeNode( "include" ) )
				{
					ImGui::Selectable( "Foo.h" );
					ImGui::TreePop();
				}

				if( ImGui::TreeNode( "src" ) )
				{
					ImGui::Selectable( "Foo.cpp" );
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
}
