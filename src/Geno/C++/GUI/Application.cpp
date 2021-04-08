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

//////////////////////////////////////////////////////////////////////////

Application::~Application( void )
{
	// Save workspace on exit
	CloseWorkspace();

} // ~Application

//////////////////////////////////////////////////////////////////////////

int Application::Run( void )
{
	MainWindow::Instance().Init();

	while( MainWindow::Instance().BeginFrame() )
	{
		MainMenuBar::Instance().Show();

		// This will update all modals recursively
		if( !m_ModalStack.empty() )
			m_ModalStack.front()->Update();

		MainWindow::Instance().EndFrame();
	}

	return 0;

} // Run

//////////////////////////////////////////////////////////////////////////

void Application::NewWorkspace( std::filesystem::path Location, std::string Name )
{
	CloseWorkspace();

	Workspace& rWorkspace = m_CurrentWorkspace.emplace( std::move( Location ) );
	rWorkspace.m_Name      = std::move( Name );

} // NewWorkspace

//////////////////////////////////////////////////////////////////////////

void Application::LoadWorkspace( const std::filesystem::path& rPath )
{
	CloseWorkspace();
	NewWorkspace( rPath.parent_path(), rPath.filename().replace_extension().string() );

	m_CurrentWorkspace->Deserialize();

} // LoadWorkspace

//////////////////////////////////////////////////////////////////////////

void Application::CloseWorkspace( void )
{
	if( m_CurrentWorkspace )
		m_CurrentWorkspace->Serialize();

	m_CurrentWorkspace.reset();

} // CloseWorkspace

//////////////////////////////////////////////////////////////////////////

void Application::PushModal( IModal* pModal )
{
	m_ModalStack.push_back( pModal );

} // PushModal

//////////////////////////////////////////////////////////////////////////

void Application::PopModal( void )
{
	m_ModalStack.pop_back();

} // PopModal

//////////////////////////////////////////////////////////////////////////

IModal* Application::NextModal( IModal* pPrevious )
{
	if( auto it = std::find( m_ModalStack.begin(), m_ModalStack.end(), pPrevious ); it != m_ModalStack.end() )
	{
		if( ++it == m_ModalStack.end() )
			return nullptr;

		return *it;
	}

	return nullptr;

} // NextModal

//////////////////////////////////////////////////////////////////////////

Workspace* Application::CurrentWorkspace( void )
{
	return m_CurrentWorkspace.has_value() ? &m_CurrentWorkspace.value() : nullptr;

} // CurrentWorkspace
