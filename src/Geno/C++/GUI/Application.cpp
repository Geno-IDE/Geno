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

#include "Application.h"

#include "GUI/Modals/IModal.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "Misc/Settings.h"

#include <iostream>

int Application::Run( void )
{
	MainWindow::Instance().Init();

	while( MainWindow::Instance().BeginFrame() )
	{
		MainMenuBar::Instance().Show();

		// This will update all modals recursively
		if( !modal_stack_.empty() )
			modal_stack_.front()->Update();

		MainWindow::Instance().EndFrame();
	}

	return 0;
}

void Application::NewWorkspace( const std::filesystem::path& where )
{
	CloseWorkspace();

	Workspace& workspace = current_workspace_.emplace( where.parent_path() );
	workspace.name_      = where.filename().replace_extension().string();
}

void Application::LoadWorkspace( const std::filesystem::path& path )
{
	CloseWorkspace();
	NewWorkspace( path );

	current_workspace_->Deserialize();
}

void Application::CloseWorkspace( void )
{
	if( current_workspace_ )
		current_workspace_->Serialize();

	current_workspace_.reset();
}

void Application::PushModal( IModal* modal )
{
	modal_stack_.push_back( modal );
}

void Application::PopModal( void )
{
	modal_stack_.pop_back();
}

IModal* Application::NextModal( IModal* previous )
{
	if( auto it = std::find( modal_stack_.begin(), modal_stack_.end(), previous ); it != modal_stack_.end() )
	{
		if( ++it == modal_stack_.end() )
			return nullptr;

		return *it;
	}

	return nullptr;
}

Workspace* Application::CurrentWorkspace( void )
{
	return current_workspace_.has_value() ? &current_workspace_.value() : nullptr;
}
