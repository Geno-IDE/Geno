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

#pragma once
#include <functional>
#include <imgui.h>
#include <string>

class Texture2D;

//////////////////////////////////////////////////////////////////////////

namespace ImGuiAux
{
struct ButtonData
{
	float  Rounding     = 10.0f;
	ImVec4 Color        = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f );
	ImVec4 ColorHovered = ImVec4( 0.2f, 0.4f, 0.67f, 0.4f );
	ImVec4 ColorText    = ImVec4( 0.2f, 0.6f, 0.8f, 1.0f );
	ImVec2 Size         = ImVec2( 0, 0 );
};

extern void RenameTree          ( std::string& rNameToRename, bool& rRename, const std::function< bool( void ) >& rCallback );
extern bool PushTreeWithIcon    ( const char* pLabel, const Texture2D& rTexture, bool Rename, bool DrawArrow = true, bool ForceHovered = false );
extern bool BeginChildHorizontal( const ImGuiID ID, const ImVec2& rSize, const bool Border = false, const ImGuiWindowFlags Flags = 0 );
extern void TextCentered        ( const char* pText );
extern bool Button              ( const char* pLabel, const ButtonData& ButtonData );

} // namespace ImGuiAux
