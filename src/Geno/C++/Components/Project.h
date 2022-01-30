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

#include "Components/Configuration.h"
#include "Components/INode.h"

#include <Common/Event.h>
#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

#include <filesystem>
#include <vector>

class ICompiler;

class File : public INode
{
public:
	File( std::filesystem::path Location, std::string Name );

	void Rename( std::string Name ) override;

}; // File

//////////////////////////////////////////////////////////////////////////

class Group : public INode
{
public:
	Group( std::filesystem::path Location, std::string Name, bool WorkspaceGroup = false );

	void Rename( std::string Name ) override;

	void Serialize( GCL::Serializer& rSerializer );
	void Deserialize( GCL::Member& rGroupMember );

	bool m_WorkspaceGroup = false;

}; // Group

//////////////////////////////////////////////////////////////////////////

class Project : public INode
{
	GENO_DISABLE_COPY( Project );

//////////////////////////////////////////////////////////////////////////

public:

	enum class Kind
	{
		Unspecified,
		Application,
		StaticLibrary,
		DynamicLibrary,

	}; // Kind


//////////////////////////////////////////////////////////////////////////

	static constexpr std::string_view EXTENSION = ".gprj";

//////////////////////////////////////////////////////////////////////////

	explicit Project( std::filesystem::path Location, std::string Name );

//////////////////////////////////////////////////////////////////////////

	bool Serialize  ( void );
	bool Deserialize( void );

//////////////////////////////////////////////////////////////////////////

	void Rename( std::string Name ) override;

	std::vector< std::filesystem::path > FindSourceFolders( void );

//////////////////////////////////////////////////////////////////////////

	struct
	{
		Event< Project, void( std::filesystem::path OutputFile, bool Success ) > BuildFinished;

	} Events;

//////////////////////////////////////////////////////////////////////////

	Kind          m_ProjectKind = Kind::Application;
	Configuration m_LocalConfiguration;

}; // Project
