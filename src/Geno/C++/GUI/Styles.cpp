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

#include "Styles.h"

#include <imgui.h>

//////////////////////////////////////////////////////////////////////////

static void CommonVars( ImGuiStyle& rStyle )
{
	rStyle.FrameRounding            = 0;
	rStyle.WindowPadding            = ImVec2( 2, 2 );
	rStyle.FramePadding             = ImVec2( 2, 2 );
	rStyle.ItemSpacing              = ImVec2( 4, 1 );
	rStyle.ItemInnerSpacing         = ImVec2( 2, 0 );
	rStyle.ScrollbarSize            = 10;
	rStyle.WindowRounding           = 2;
	rStyle.ChildRounding            = 0;
	rStyle.FrameRounding            = 0;
	rStyle.PopupRounding            = 2;
	rStyle.ScrollbarRounding        = 2;
	rStyle.TabRounding              = 2;
	rStyle.WindowTitleAlign         = ImVec2( 0.5f, 0.5f );
	rStyle.WindowMenuButtonPosition = ImGuiDir_None;
	rStyle.ButtonTextAlign          = ImVec2( 0.5f, 0.5f );
	rStyle.SelectableTextAlign      = ImVec2( 0, 0 );
	rStyle.DisplaySafeAreaPadding   = ImVec2( 0, 0 );

} // CommonVars

//////////////////////////////////////////////////////////////////////////

static void CommonColors( ImGuiStyle& rStyle )
{
	rStyle.Colors[ ImGuiCol_TitleBgActive      ] = rStyle.Colors[ ImGuiCol_TitleBg ];
	rStyle.Colors[ ImGuiCol_TabUnfocused       ] = rStyle.Colors[ ImGuiCol_Tab ];
	rStyle.Colors[ ImGuiCol_TabUnfocusedActive ] = rStyle.Colors[ ImGuiCol_TabActive ];

} // CommonColors

//////////////////////////////////////////////////////////////////////////

void Styles::Light( void )
{
	ImGuiStyle& rStyle = ImGui::GetStyle();

	// Vars
	{
		CommonVars( rStyle );
	}

	// Colors
	{
		ImGui::StyleColorsLight( &rStyle );

		rStyle.Colors[ ImGuiCol_WindowBg          ] = ImColor( 0xFFECECEC );
		rStyle.Colors[ ImGuiCol_PopupBg           ] = ImColor( 0xFFECECEC );
		rStyle.Colors[ ImGuiCol_FrameBg           ] = ImColor( 0xFFFAFAFA );
		rStyle.Colors[ ImGuiCol_FrameBgHovered    ] = ImColor( 0x66FAFAFA );
		rStyle.Colors[ ImGuiCol_FrameBgActive     ] = ImColor( 0xABFAFAFA );
		rStyle.Colors[ ImGuiCol_CheckMark         ] = ImColor( 0xFF3F3F3F );
		rStyle.Colors[ ImGuiCol_SliderGrab        ] = ImColor( 0xC7CCCCCC );
		rStyle.Colors[ ImGuiCol_SliderGrabActive  ] = ImColor( 0x99CCCCCC );
		rStyle.Colors[ ImGuiCol_Button            ] = ImColor( 0x66FFFFFF );
		rStyle.Colors[ ImGuiCol_ButtonHovered     ] = ImColor( 0x99C4C4C4 );
		rStyle.Colors[ ImGuiCol_ButtonActive      ] = ImColor( 0xFFFFFFFF );
		rStyle.Colors[ ImGuiCol_Header            ] = ImColor( 0x4FFAFAFA );
		rStyle.Colors[ ImGuiCol_HeaderHovered     ] = ImColor( 0xCCC4C4C4 );
		rStyle.Colors[ ImGuiCol_HeaderActive      ] = ImColor( 0xFFFAFAFA );
		rStyle.Colors[ ImGuiCol_SeparatorHovered  ] = ImColor( 0xC7CCCCCC );
		rStyle.Colors[ ImGuiCol_SeparatorActive   ] = ImColor( 0xFFCCCCCC );
		rStyle.Colors[ ImGuiCol_ResizeGrip        ] = ImColor( 0x7F3F3F3F );
		rStyle.Colors[ ImGuiCol_ResizeGripHovered ] = ImColor( 0x9F3F3F3F );
		rStyle.Colors[ ImGuiCol_ResizeGripActive  ] = ImColor( 0xBF3F3F3F );
		rStyle.Colors[ ImGuiCol_Tab               ] = ImColor( 0x7FCCCCCC );
		rStyle.Colors[ ImGuiCol_TabHovered        ] = ImColor( 0x7FEAEAEA );
		rStyle.Colors[ ImGuiCol_TabActive         ] = ImColor( 0x7FFFFFFF );
		rStyle.Colors[ ImGuiCol_DockingPreview    ] = ImColor( 0x37C4C4C4 );
		rStyle.Colors[ ImGuiCol_TextSelectedBg    ] = ImColor( 0x59C4C4C4 );
		rStyle.Colors[ ImGuiCol_DragDropTarget    ] = ImColor( 0xF2FAFAFA );
		rStyle.Colors[ ImGuiCol_NavHighlight      ] = ImColor( 0xCCFAFAFA );

		CommonColors( rStyle );
	}

} // Light

//////////////////////////////////////////////////////////////////////////

void Styles::Dark( void )
{
	ImGuiStyle& rStyle = ImGui::GetStyle();

	// Vars
	{
		CommonVars( rStyle );
	}

	// Colors
	{
		ImGui::StyleColorsDark( &rStyle );

		rStyle.Colors[ ImGuiCol_FrameBg           ] = ImColor( 0xF0191919 );
		rStyle.Colors[ ImGuiCol_Tab               ] = ImColor( 0x7F404040 );
		rStyle.Colors[ ImGuiCol_TabHovered        ] = ImColor( 0x9F808080 );
		rStyle.Colors[ ImGuiCol_TabActive         ] = ImColor( 0xFF404040 );
		rStyle.Colors[ ImGuiCol_DockingPreview    ] = ImColor( 0xB35B5B5B );
		rStyle.Colors[ ImGuiCol_CheckMark         ] = ImColor( 0xFFD8D8D8 );
		rStyle.Colors[ ImGuiCol_SliderGrab        ] = ImColor( 0xFFAAAAAA );
		rStyle.Colors[ ImGuiCol_SliderGrabActive  ] = ImColor( 0xFFD6D6D6 );
		rStyle.Colors[ ImGuiCol_Button            ] = ImColor( 0xB4787878 );
		rStyle.Colors[ ImGuiCol_ButtonHovered     ] = ImColor( 0xFF787878 );
		rStyle.Colors[ ImGuiCol_ButtonActive      ] = ImColor( 0xFF939393 );
		rStyle.Colors[ ImGuiCol_Header            ] = ImColor( 0x4F676767 );
		rStyle.Colors[ ImGuiCol_HeaderHovered     ] = ImColor( 0xCC676767 );
		rStyle.Colors[ ImGuiCol_HeaderActive      ] = ImColor( 0xFF676767 );
		rStyle.Colors[ ImGuiCol_Separator         ] = ImColor( 0x80808080 );
		rStyle.Colors[ ImGuiCol_SeparatorHovered  ] = ImColor( 0xC7BFBFBF );
		rStyle.Colors[ ImGuiCol_SeparatorActive   ] = ImColor( 0xFFBFBFBF );
		rStyle.Colors[ ImGuiCol_ResizeGrip        ] = ImColor( 0x40FAFAFA );
		rStyle.Colors[ ImGuiCol_ResizeGripHovered ] = ImColor( 0x89FAFAFA );
		rStyle.Colors[ ImGuiCol_ResizeGripActive  ] = ImColor( 0xC8FAFAFA );
		rStyle.Colors[ ImGuiCol_TextSelectedBg    ] = ImColor( 0x30FAFAFA );
		rStyle.Colors[ ImGuiCol_NavHighlight      ] = ImColor( 0xFFFAFAFA );

		CommonColors( rStyle );
	}

} // Dark
