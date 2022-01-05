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
#include "Components/BuildMatrix.h"

#include <string_view>
#include <vector>

#include <Common/Macros.h>
#include <Common/Texture2D.h>

class  Drop;
struct CompilationDone;
struct WorkspaceOpened;

class TitleBar
{
public:

	TitleBar( void );

//////////////////////////////////////////////////////////////////////////

	void Draw( void );

//////////////////////////////////////////////////////////////////////////

	float Height( void ) const { return m_Height; }

//////////////////////////////////////////////////////////////////////////

	bool ShowTextEdit              = false;
	bool ShowDemoWindow            = false;
	bool ShowAboutWindow           = false;
	bool ShowOutputWindow          = false;
	bool ShowWorkspaceOutliner     = false;
	bool ShowGenoDiscordSettings   = false;
	bool ShowFindInWorkspaceWindow = false;

//////////////////////////////////////////////////////////////////////////

private:

	void ActionFileNewWorkspace       ( void );
	void ActionExtShowGenoDiscord     ( void );
	void ActionFileOpenWorkspace      ( void );
	void ActionFileOpenRecentWorkspace( std::filesystem::path Path );
	void ActionFileCloseWorkspace     ( void );
	void ActionBuildBuildAndRun       ( void );
	void ActionBuildBuild             ( void );
	void AddBuildMatrixColumn         ( BuildMatrix::Column& rColumn );

//////////////////////////////////////////////////////////////////////////

	Texture2D m_IconTexture = { };

	float     m_Height      = 0.0f;

}; // TitleBar
