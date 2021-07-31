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
#include "Common/Macros.h"
#include "Common/Texture2D.h"

#include "WidgetCommands/CommandStack.h"

#include <filesystem>
#include <string>

class WorkspaceOutliner
{
public:

	WorkspaceOutliner( void );

//////////////////////////////////////////////////////////////////////////

	void Show( bool* pOpen );

//////////////////////////////////////////////////////////////////////////

private:

	Texture2D             m_IconTextureWorkspace    = { };
	Texture2D             m_IconTextureProject      = { };
	Texture2D             m_IconTextureFileFilter   = { };
	Texture2D             m_IconTextureSourceFile   = { };

	std::filesystem::path m_SelectedFile            = { };
	std::filesystem::path m_SelectedFileFilterName  = { };
	std::string           m_RenameText              = { };
	std::string           m_SelectedProjectName     = { };
	std::string           m_ProjectNodeToBeExpanded = { };

	bool                  m_ExpandWorkspaceNode     = false;

	CommandStack m_UndoCommandStack = {};
	CommandStack m_RedoCommandStack = {};

}; // WorkspaceWidget
