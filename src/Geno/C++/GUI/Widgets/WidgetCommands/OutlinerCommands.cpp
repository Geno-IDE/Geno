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

#include "OutlinerCommands.h"

#include "Application.h"
#include "Components/Project.h"
#include "GUI/MainWindow.h"
#include "GUI/Widgets/TextEdit.h"

//////////////////////////////////////////////////////////////////////////

OutlinerCommands::RenameNodeCommand::RenameNodeCommand( std::string NewName, INode* pNode )
	: m_NewName( std::move( NewName ) )
	, m_pNode( std::move( pNode ) )
{
} // RenameNodeCommand

//////////////////////////////////////////////////////////////////////////

void OutlinerCommands::RenameNodeCommand::Execute( void )
{
	m_PreviousName = m_pNode->m_Name;
	m_pNode->Rename( m_NewName );

} // RenameNodeCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::RenameNodeCommand::Undo( void )
{
	const std::string Temp = m_pNode->m_Name;
	if( m_PreviousName.empty() )
		m_PreviousName = m_NewName;
	m_NewName = m_PreviousName;

	Execute();

	return new RenameNodeCommand( Temp, m_pNode );

} // RenameNodeCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::RenameNodeCommand::Redo( void )
{
	Execute();

	return new RenameNodeCommand( m_PreviousName, m_pNode );

} // RenameNodeCommand::Redo

//////////////////////////////////////////////////////////////////////////

OutlinerCommands::NewNodeCommand::NewNodeCommand( NodeKind NewNodeKind, std::string Name, std::filesystem::path Location, INode* pParentNode )
	: m_NewNodeKind( std::move( NewNodeKind ) )
	, m_Name( std::move( Name ) )
	, m_Location( std::move( Location ) )
	, m_pParentNode( std::move( pParentNode ) )
{
} // NewNodeCommand

//////////////////////////////////////////////////////////////////////////

void OutlinerCommands::NewNodeCommand::Execute( void )
{
	switch( m_NewNodeKind )
	{
		case NodeKind::Workspace:
			break;
		case NodeKind::Project:
		{
			Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
			pWorkspace->NewProject( m_Location, m_Name );
		}
		break;
		case NodeKind::FileFilter:
		{
			if( !m_pParentNode->ChildByName( m_Name ) )
				m_pParentNode->AddChild( new FileFilter( m_Name ) );
		}
		break;
		case NodeKind::File:
		{
			FileFilter* pFileFilter = ( FileFilter* )m_pParentNode;
			pFileFilter->NewFile( m_Location, m_Name );
		}
		break;
	}

} // NewNodeCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::NewNodeCommand::Undo( void )
{
	m_pParentNode->RemoveChild( m_Name );
	return new NewNodeCommand( m_NewNodeKind, m_Name, m_Location, m_pParentNode );

} // NewNodeCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::NewNodeCommand::Redo( void )
{
	AddRemovedNode( m_pParentNode, m_Name );
	return new NewNodeCommand( m_NewNodeKind, m_Name, m_Location, m_pParentNode );

} // NewNodeCommand::Redo

//////////////////////////////////////////////////////////////////////////

OutlinerCommands::AddNodeCommand::AddNodeCommand( NodeKind AddNodeKind, std::string Name, std::filesystem::path Location, INode* pParentNode )
	: m_AddNodeKind( std::move( AddNodeKind ) )
	, m_Name( std::move( Name ) )
	, m_Location( std::move( Location ) )
	, m_pParentNode( std::move( pParentNode ) )
{
} // AddNodeCommand

//////////////////////////////////////////////////////////////////////////

void OutlinerCommands::AddNodeCommand::Execute( void )
{
	switch( m_AddNodeKind )
	{
		case NodeKind::Workspace:
			break;
		case NodeKind::Project:
		{
			Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
			pWorkspace->AddProject( m_Location / m_Name );
		}
		break;
		case NodeKind::File:
		{
			if( !m_pParentNode->ChildByName( m_Name ) )
				m_pParentNode->AddChild( new File( m_Location, m_Name ) );
		}
		break;
		case NodeKind::FileFilter:
			break;
	}

} // AddNodeCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::AddNodeCommand::Undo( void )
{
	m_pParentNode->RemoveChild( m_Name );
	return new AddNodeCommand( m_AddNodeKind, m_Name, m_Location, m_pParentNode );

} // AddNodeCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::AddNodeCommand::Redo( void )
{
	AddRemovedNode( m_pParentNode, m_Name );
	return new AddNodeCommand( m_AddNodeKind, m_Name, m_Location, m_pParentNode );

} // AddNodeCommand::Redo

//////////////////////////////////////////////////////////////////////////

OutlinerCommands::RemoveNodeCommand::RemoveNodeCommand( std::string Name, INode* pParentNode )
	: m_Name( std::move( Name ) )
	, m_pParentNode( std::move( pParentNode ) )
{
} // RemoveNodeCommand

//////////////////////////////////////////////////////////////////////////

void OutlinerCommands::RemoveNodeCommand::Execute( void )
{
	m_pParentNode->RemoveChild( m_Name );

} // RemoveNodeCommand::Execute

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::RemoveNodeCommand::Undo( void )
{
	AddRemovedNode( m_pParentNode, m_Name );
	return new RemoveNodeCommand( m_Name, m_pParentNode );

} // RemoveNodeCommand::Undo

//////////////////////////////////////////////////////////////////////////

ICommand* OutlinerCommands::RemoveNodeCommand::Redo( void )
{
	Execute();
	return new RemoveNodeCommand( m_Name, m_pParentNode );

} // RemoveNodeCommand::Redo
