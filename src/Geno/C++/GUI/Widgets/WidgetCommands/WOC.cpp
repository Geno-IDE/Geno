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

#include "WOC.h"

#include "Application.h"
#include "GUI/MainWindow.h"
#include "GUI/Widgets/TextEdit.h"

//////////////////////////////////////////////////////////////////////////

WOC::RenameItemCommand::RenameItemCommand( ItemType RenameItemType, const std::filesystem::path& rPreviousName, const std::filesystem::path& rNewName, const std::string& rProjectName, const std::filesystem::path& rFileFilterName )
	: m_RenameItemType( RenameItemType )
	, m_PreviousName  ( rPreviousName )
	, m_NewName       ( rNewName )
	, m_FileFilterName( rFileFilterName )
	, m_ProjectName   ( rProjectName )
{
} // RenameItemCommand

//////////////////////////////////////////////////////////////////////////

void WOC::RenameItemCommand::Execute( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	switch( m_RenameItemType )
	{
		case ItemType_Workspace:
			pWorkspace->Rename( m_NewName.string() );
			break;
		case ItemType_Project:
			pWorkspace->RenameProject( m_PreviousName.string(), m_NewName.string() );
			break;
		case ItemType_FileFilter:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RenameFileFilter( m_PreviousName, m_NewName.string() );
			}
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RenameFile( m_PreviousName, m_FileFilterName, m_NewName.filename().string() );
				MainWindow::Instance().pTextEdit->ReplaceFile( m_PreviousName, m_NewName );
			}
			break;
	}

} // RenameItemCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::RenameItemCommand::Undo( void )
{
	// m_PreviousName -> Used For Locating Project/FileFilter/File
	// m_NewName      -> Used As Rename String

	auto Temp      = m_NewName; // As We Now Have To Find The Item With The Renamed Name
	m_NewName      = m_PreviousName; // Set The Name Before Rename For Undo
	m_PreviousName = Temp;

	Execute();

	return new RenameItemCommand( m_RenameItemType, m_NewName, m_PreviousName, m_ProjectName, m_FileFilterName );

} // RenameItemCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::RenameItemCommand::Redo( void )
{
	Execute();
	return new RenameItemCommand( m_RenameItemType, m_PreviousName, m_NewName, m_ProjectName, m_FileFilterName );

} // RenameItemCommand::Redo

//////////////////////////////////////////////////////////////////////////

WOC::NewItemCommand::NewItemCommand( ItemType NewItemType, const std::filesystem::path& rLocation, const std::string& rName, const std::string& rProjectName, const std::filesystem::path& rFileFilterName )
	: m_NewItemType   ( NewItemType )
	, m_Location      ( rLocation )
	, m_Name          ( rName )
	, m_FileFilterName( rFileFilterName )
	, m_ProjectName   ( rProjectName )
{
} // NewItemCommand

//////////////////////////////////////////////////////////////////////////

void WOC::NewItemCommand::Execute( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	switch( m_NewItemType )
	{
		case ItemType_Project:
			pWorkspace->NewProject( m_Location, m_Name );
			break;
		case ItemType_FileFilter:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->NewFileFilter( m_Name );
			}
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->NewFile( m_Location / m_Name, m_FileFilterName );
			}
			break;
	}

} // NewItemCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::NewItemCommand::Undo( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	switch( m_NewItemType )
	{
		case ItemType_Project:
			pWorkspace->RemoveProject( m_Name );
			break;
		case ItemType_FileFilter:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RemoveFileFilter( m_Name );
			}
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RemoveFile( m_Location / m_Name, m_FileFilterName );
			}
			break;
	}

	return new NewItemCommand( m_NewItemType, m_Location, m_Name, m_ProjectName, m_FileFilterName );

} // NewItemCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::NewItemCommand::Redo( void )
{
	Execute();
	return new NewItemCommand( m_NewItemType, m_Location, m_Name, m_ProjectName, m_FileFilterName );

} // NewItemCommand::Redo

//////////////////////////////////////////////////////////////////////////

WOC::AddItemCommand::AddItemCommand( ItemType AddItemType, const std::filesystem::path& rPath, const std::string& rProjectName, const std::filesystem::path& rFileFilterName )
	: m_AddItemType   ( AddItemType )
	, m_Path          ( rPath )
	, m_FileFilterName( rFileFilterName )
	, m_ProjectName   ( rProjectName )
{
} // AddItemCommand

//////////////////////////////////////////////////////////////////////////

void WOC::AddItemCommand::Execute( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();

	switch( m_AddItemType )
	{
		case ItemType_Project:
			pWorkspace->AddProject( m_Path );
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->AddFile( m_Path, m_FileFilterName );
			}
			break;
	}

} // AddItemCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::AddItemCommand::Undo( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();

	switch( m_AddItemType )
	{
		case ItemType_Project:
			pWorkspace->RemoveProject( m_Path.stem().string() );
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RemoveFile( m_Path, m_FileFilterName );
			}
			break;
	}

	return new AddItemCommand( m_AddItemType, m_Path, m_ProjectName, m_FileFilterName );

} // AddItemCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::AddItemCommand::Redo( void )
{
	Execute();
	return new AddItemCommand( m_AddItemType, m_Path, m_ProjectName, m_FileFilterName );

} // AddItemCommand::Redo

//////////////////////////////////////////////////////////////////////////

WOC::RemoveItemCommand::RemoveItemCommand( ItemType RemoveItemType, const std::filesystem::path& rName, const std::string& rProjectName, const std::filesystem::path& rFileFilterName )
	: m_RemoveItemType( RemoveItemType )
	, m_Name          ( rName )
	, m_FileFilterName( rFileFilterName )
	, m_ProjectName   ( rProjectName )
{
} // RemoveItemCommand

//////////////////////////////////////////////////////////////////////////

void WOC::RemoveItemCommand::Execute( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();

	switch( m_RemoveItemType )
	{
		case ItemType_Project:
			pWorkspace->RemoveProject( m_Name.stem().string() );
			break;
		case ItemType_FileFilter:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				m_FileFilter = *pProject->FileFilterByName( m_Name );
				pProject->RemoveFileFilter( m_Name );
			}
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->RemoveFile( m_Name, m_FileFilterName );
			}
			break;
	}

} // RemoveItemCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::RemoveItemCommand::Undo( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();

	switch( m_RemoveItemType )
	{
		case ItemType_Project:
			pWorkspace->AddProject( m_Name );
			break;
		case ItemType_FileFilter:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->m_FileFilters.emplace_back( std::move( m_FileFilter ) );
			}
			break;
		case ItemType_File:
			if( Project* pProject = pWorkspace->ProjectByName( m_ProjectName ) )
			{
				pProject->AddFile( m_Name, m_FileFilterName );
			}
			break;
	}

	return new RemoveItemCommand( m_RemoveItemType, m_Name, m_ProjectName, m_FileFilterName );

} // RemoveItemCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* WOC::RemoveItemCommand::Redo( void )
{
	Execute();
	return new RemoveItemCommand( m_RemoveItemType, m_Name, m_ProjectName, m_FileFilterName );

} // RemoveItemCommand::Redo
