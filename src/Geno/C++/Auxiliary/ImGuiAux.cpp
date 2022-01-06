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
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

void ImGuiAux::RenameTree( std::string& rNameToRename, bool& rRename, const std::function< bool( void ) >& rCallback )
{
	bool HighlightBorder;

	if( rNameToRename.empty() )
		HighlightBorder = true;
	else
		HighlightBorder = false;

	ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x );

	auto& BgColor = ImGui::GetStyle().Colors[ ImGuiCol_WindowBg ];

	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 0.0f );
	ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1.5f );
	ImGui::PushStyleColor( ImGuiCol_FrameBg, BgColor );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, BgColor );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, BgColor );

	if( HighlightBorder )
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 1.0f, 0.0, 0.0, 1.0f ) );

	bool IsEnterPressed = ImGui::InputText( "##Rename", &rNameToRename, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );

	ImGui::PopStyleVar( 2 );
	ImGui::PopStyleColor( HighlightBorder ? 4 : 3 );

	if( IsEnterPressed && !HighlightBorder )
	{
		if( rCallback() )
			rRename = false;
	}
	else if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Escape ) ) )
	{
		rRename = false;
	}
	else if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) || ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
	{
		if( !( ImGui::IsItemClicked( ImGuiMouseButton_Right ) || ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) )
		{
			rRename = false;
		}
	}

} //RenameTree

//////////////////////////////////////////////////////////////////////////

bool ImGuiAux::PushTreeWithIcon( const char* pLabel, const Texture2D& rTexture, bool Rename, const bool HighlightBg, bool* pExpand, const bool DrawArrow )
{
	const float   Height    = ImGui::GetFontSize();
	ImGuiWindow*  pWindow   = ImGui::GetCurrentWindow();
	ImGuiStyle&   rStyle    = ImGui::GetStyle();
	const ImGuiID ID        = pWindow->GetID( pLabel );
	ImVec2        CursorPos = pWindow->DC.CursorPos;
	const ImRect  Bounds    = ImRect( ImVec2( 0, CursorPos.y ), ImVec2( CursorPos.x + ImGui::GetContentRegionAvail().x, CursorPos.y + Height + rStyle.FramePadding.y * 2 ) );
	const bool    Opened    = ImGui::TreeNodeBehaviorIsOpen( ID );

	// Button logic and draw background
	{
		bool Hovered;
		bool Held;

		if( !Rename )
		{
			if( ImGui::ButtonBehavior( Bounds, ID, &Hovered, &Held, true ) )
			{
				if( pExpand )
					*pExpand = *pExpand ? false : true;
				pWindow->DC.StateStorage->SetInt( ID, Opened ? 0 : 1 );
			}

			if( !HighlightBg )
			{
				if( Hovered || Held )
					pWindow->DrawList->AddRectFilled( Bounds.Min, Bounds.Max, ImGui::GetColorU32( Held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered ) );
			}
			else
			{
				pWindow->DrawList->AddRectFilled( Bounds.Min, Bounds.Max, ImGui::ColorConvertFloat4ToU32( { 0.2f, 0.6f, 0.8f, 1.0f } ) );
			}
		}
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

		if( !Rename )
			ImGui::RenderText( Pos, pLabel );
	}

	if( !Rename )
	{
		ImGui::ItemSize( Bounds, rStyle.FramePadding.y );
		ImGui::ItemAdd( Bounds, ID );
	}

	float Offset = ImGui::GetCursorPosX() + 25.0f; // 25 As Image Size

	if( DrawArrow )
		Offset += ImGui::GetFontSize(); // If The Tree Have Arrow Add ArrowSize = FontSize

	if( Opened )
		ImGui::TreePush( pLabel );

	if( Rename )
		ImGui::SetCursorPosX( Offset );

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
	ImGui::TextUnformatted( pText );

} // TextCentered

//////////////////////////////////////////////////////////////////////////

bool ImGuiAux::Button( const char* pLabel, const ButtonData& ButtonData )
{
	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, ButtonData.Rounding );
	ImGui::PushStyleColor( ImGuiCol_Button, ButtonData.Color );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ButtonData.ColorHovered );
	ImGui::PushStyleColor( ImGuiCol_Text, ButtonData.ColorText );

	bool IsClicked = ImGui::Button( pLabel, ButtonData.Size );

	ImGui::PopStyleColor( 3 );
	ImGui::PopStyleVar();

	return IsClicked;
} //Button
