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
#include "ICommand.h"

#include "Components/Project.h"

#include <filesystem>
#include <string>

namespace WOC /*WOC = WorkspaceOutlinerCommands*/
{

enum ItemType_
{
	ItemType_File       = 0,
	ItemType_FileFilter = 1,
	ItemType_Project    = 2,
	ItemType_Workspace  = 3

}; // ItemType_

using ItemType = int;

//////////////////////////////////////////////////////////////////////////

class RenameItemCommand : public ICommand
{
public:

	RenameItemCommand( ItemType RenameItemType, const std::filesystem::path& rPreviousName, const std::filesystem::path& rNewName, const std::string& rProjectName = {}, const std::filesystem::path& rFileFilterName = {} );

//////////////////////////////////////////////////////////////////////////

	void Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	ItemType m_RenameItemType;

	std::filesystem::path m_PreviousName   = {};
	std::filesystem::path m_NewName        = {};
	std::filesystem::path m_FileFilterName = {}; // This Will Be Used Only In Case Of File
	std::string           m_ProjectName    = {}; // This Will Be Used Only In Case Of File/FileFilter

}; // RenameItemCommand

//////////////////////////////////////////////////////////////////////////

class NewItemCommand : public ICommand
{
public:

	NewItemCommand( ItemType NewItemType, const std::filesystem::path& rLocation, const std::string& rName, const std::string& rProjectName = {}, const std::filesystem::path& rFileFilterName = {} );

//////////////////////////////////////////////////////////////////////////

	void Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	ItemType m_NewItemType;

	std::filesystem::path m_Location = {};
	std::string           m_Name     = {};

	std::filesystem::path m_FileFilterName = {}; // This Will Be Used Only In Case Of File
	std::string           m_ProjectName    = {}; // This Will Be Used Only In Case Of File/FileFilter

}; // NewItemCommand

//////////////////////////////////////////////////////////////////////////

class AddItemCommand : public ICommand
{
public:

	AddItemCommand( ItemType AddItemType, const std::filesystem::path& rPath, const std::string& rProjectName = {}, const std::filesystem::path& rFileFilterName = {} );

//////////////////////////////////////////////////////////////////////////

	void Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	ItemType m_AddItemType;

	std::filesystem::path m_Path           = {};
	std::filesystem::path m_FileFilterName = {}; // This Will Be Used Only In Case Of File
	std::string           m_ProjectName    = {}; // This Will Be Used Only In Case Of File/FileFilter

}; // AddItemCommand

//////////////////////////////////////////////////////////////////////////

class RemoveItemCommand : public ICommand
{
public:

	RemoveItemCommand( ItemType RemoveItemType, const std::filesystem::path& rName, const std::string& rProjectName = {}, const std::filesystem::path& rFileFilterName = {} );

//////////////////////////////////////////////////////////////////////////

	void Execute( void ) override;
	ICommand* Undo( void ) override;
	ICommand* Redo( void ) override;

//////////////////////////////////////////////////////////////////////////

private:

	ItemType m_RemoveItemType;

	std::filesystem::path m_Name           = {};
	std::filesystem::path m_FileFilterName = {}; // This Will Be Used Only In Case Of File
	std::string           m_ProjectName    = {}; // This Will Be Used Only In Case Of File/FileFilter

	FileFilter m_FileFilter = {};

}; // RemoveItemCommand

} // namespace WOC
