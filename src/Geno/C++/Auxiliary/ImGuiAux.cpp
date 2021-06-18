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

#include "Auxiliary/ImGuiAux.h"

#include "Common/Texture2D.h"

#include <imgui_internal.h>

//////////////////////////////////////////////////////////////////////////

bool ImGuiAux::PushTreeWithIcon( const char* pLabel, const Texture2D& rTexture, const bool DrawArrow )
{
	const float   Height    = ImGui::GetFontSize();
	ImGuiWindow*  pWindow   = ImGui::GetCurrentWindow();
	ImGuiStyle&   rStyle    = ImGui::GetStyle();
	const ImGuiID ID        = pWindow->GetID( pLabel );
	ImVec2        CursorPos = pWindow->DC.CursorPos;
	const ImRect  Bounds    = ImRect( CursorPos, ImVec2( CursorPos.x + ImGui::GetContentRegionAvail().x, CursorPos.y + Height + rStyle.FramePadding.y * 2 ) );
	const bool    Opened    = ImGui::TreeNodeBehaviorIsOpen( ID );

	// Button logic and draw background
	{
		bool Hovered;
		bool Held;

		if( ImGui::ButtonBehavior( Bounds, ID, &Hovered, &Held, true ) )
			pWindow->DC.StateStorage->SetInt( ID, Opened ? 0 : 1 );

		if( Hovered || Held )
			pWindow->DrawList->AddRectFilled( Bounds.Min, Bounds.Max, ImGui::GetColorU32( Held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered ) );
	}

	// Arrow
	if( DrawArrow )
	{
		const ImVec2   Pos       = CursorPos + rStyle.FramePadding;
		const ImU32    Color     = ImGui::GetColorU32( ImGuiCol_Text );
		const ImGuiDir Direction = Opened ? ImGuiDir_Down : ImGuiDir_Right;

		ImGui::RenderArrow( pWindow->DrawList, Pos, Color, Direction, 0.75f );

		CursorPos.x += ImGui::GetFontSize();
		CursorPos.x += rStyle.FramePadding.x;
	}

	// Draw image
	{
		const ImVec2 Size = ImVec2( Height * rTexture.GetAspectRatio(), Height ) + rStyle.FramePadding * 2;

		pWindow->DrawList->AddImage( rTexture.GetID(), CursorPos, CursorPos + Size );

		CursorPos.x += Size.x;
	}

	// Label text
	{
		const ImVec2 Pos = CursorPos + rStyle.FramePadding; // + rStyle.ItemInnerSpacing;

		ImGui::RenderText( Pos, pLabel );
	}

	ImGui::ItemSize( Bounds, rStyle.FramePadding.y );
	ImGui::ItemAdd( Bounds, ID );

	if( Opened )
		ImGui::TreePush( pLabel );

	return Opened;

} // PushTreeWithIcon

//////////////////////////////////////////////////////////////////////////

bool ImGuiAux::BeginChildHorizontal( const ImGuiID ID, const ImVec2& rSize, const bool Border, const ImGuiWindowFlags Flags )
{
	if( ImGui::BeginChild( ID, rSize, Border, Flags ) )
	{
		ImGuiWindow* pWindow       = ImGui::GetCurrentWindow();
		pWindow->DC.LayoutType     = ImGuiLayoutType_Horizontal;
		pWindow->DC.CurrLineSize.y = pWindow->Size.y;

		return true;
	}

	return false;

} // BeginChildHorizontal

//////////////////////////////////////////////////////////////////////////

void ImGuiAux::TextCentered( const char* pText )
{
	const ImVec2 TextSize = ImGui::CalcTextSize( pText );

	ImGui::SetCursorPosX( ( ImGui::GetWindowWidth() - TextSize.x ) * 0.5f );
	ImGui::Text( pText );

} // TextCentered
