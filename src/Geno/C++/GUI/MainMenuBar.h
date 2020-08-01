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

#pragma once
#include "Common/Macros.h"

#include <string_view>
#include <vector>

struct CompilationDone;

class MainMenuBar
{
	GENO_SINGLETON( MainMenuBar );

public:

	void Show( void );

public:

	float Height( void ) const { return height_; }

private:

	void ActionFileNewWorkspace  ( void );
	void ActionFileOpenWorkspace ( void );
	void ActionFileCloseWorkspace( void );
	void ActionFileExit          ( void );
	void ActionBuildBuild        ( void );
	void ActionViewTextEdit      ( void );
	void ActionViewWorkspace     ( void );
	void ActionViewSettings      ( void );
	void ActionViewOutput        ( void );
	void ActionHelpDemo          ( void );
	void ActionHelpAbout         ( void );

private:

	static void OnCompilerDone( const CompilationDone& e );

private:

	float height_ = 0.f;

	bool show_demo_window_  = false;
	bool show_about_window_ = false;
	bool show_text_edit_    = true;
	bool show_workspace_    = true;
	bool show_settings_     = false;
	bool show_output_       = false;

};
