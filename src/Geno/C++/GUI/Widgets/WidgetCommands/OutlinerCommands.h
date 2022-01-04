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

#include "Components/INode.h"
#include "ICommand.h"

#include <filesystem>
#include <string>

namespace OutlinerCommands
{

class RenameNodeCommand : public ICommand
{
public:

	RenameNodeCommand( std::string NewName, INode* pNode );

//////////////////////////////////////////////////////////////////////////

	void      Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	std::string m_PreviousName = {};
	std::string m_NewName      = {};
	INode* m_pNode;

}; // RenameNodeCommand

//////////////////////////////////////////////////////////////////////////

class NewNodeCommand : public ICommand
{
public:

	NewNodeCommand( NodeKind NewNodeKind, std::string Name, std::filesystem::path Location, INode* pParentNode );

//////////////////////////////////////////////////////////////////////////

	void      Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	NodeKind              m_NewNodeKind;
	std::string           m_Name     = {};
	std::filesystem::path m_Location = {};
	INode*                m_pParentNode;

}; // NewNodeCommand

//////////////////////////////////////////////////////////////////////////

class AddNodeCommand : public ICommand
{
public:

	AddNodeCommand( NodeKind AddNodeKind, std::string Name, std::filesystem::path Location, INode* pParentNode );

//////////////////////////////////////////////////////////////////////////

	void      Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	NodeKind              m_AddNodeKind;
	std::string           m_Name     = {};
	std::filesystem::path m_Location = {};
	INode*                m_pParentNode;

}; // AddNodeCommand

//////////////////////////////////////////////////////////////////////////

class RemoveNodeCommand : public ICommand
{
public:

	RemoveNodeCommand( std::string Name, INode* pParentNode );

//////////////////////////////////////////////////////////////////////////

	void      Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	std::string m_Name = {};
	INode*      m_pParentNode;

}; // RemoveNodeCommand

} // namespace OutlinerCommands
