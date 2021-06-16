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

#include <Common/Event.h>
#include <GCL/Object.h>

#include <filesystem>
#include <vector>

class ICompiler;

class Project
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

	explicit Project( std::filesystem::path Location );
	         Project( Project&& rrOther );

	Project& operator=( Project&& rrOther );

//////////////////////////////////////////////////////////////////////////

	void Build      ( ICompiler& rCompiler );
	bool Serialize  ( void );
	bool Deserialize( void );

//////////////////////////////////////////////////////////////////////////

	struct
	{
		Event< Project, void( std::filesystem::path OutputFile, bool Success ) > BuildFinished;

	} Events;

//////////////////////////////////////////////////////////////////////////

	Kind                                 m_Kind = Kind::Application;

	std::filesystem::path                m_Location;
	std::string                          m_Name;
	std::vector< std::filesystem::path > m_Files;
	std::vector< std::filesystem::path > m_IncludeDirectories;
	std::vector< std::filesystem::path > m_LibraryDirectories;
	std::vector< std::string >           m_Defines;
	std::vector< std::string >           m_Libraries;
	std::vector< std::filesystem::path > m_FilesLeftToBuild;
	std::vector< std::filesystem::path > m_FilesToLink;

//////////////////////////////////////////////////////////////////////////

private:

	static void GCLObjectCallback( GCL::Object Object, void* pUser );

//////////////////////////////////////////////////////////////////////////

private:

	void BuildNextFile( ICompiler& rCompiler );
	void Link         ( ICompiler& rCompiler );

}; // Project
