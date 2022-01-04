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
#include "Compilers/ICompiler.h"
#include "Components/BuildMatrix.h"
#include "Components/INode.h"
#include "Components/Project.h"

#include <Common/Event.h>
#include <Common/Process.h>
#include <GCL/Deserializer.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class Workspace;

class Workspace : public INode
{
	GENO_DISABLE_COPY( Workspace );
	GENO_DEFAULT_MOVE( Workspace );

//////////////////////////////////////////////////////////////////////////

public:

	static constexpr std::string_view EXTENSION = ".gwks";

//////////////////////////////////////////////////////////////////////////

	explicit Workspace( std::string Name, std::filesystem::path Location );

//////////////////////////////////////////////////////////////////////////

	void Build      ( void );
	bool Serialize  ( void );
	bool Deserialize( void );

//////////////////////////////////////////////////////////////////////////

	void     Rename( std::string Name ) override;
	Project* NewProject( std::filesystem::path Location, std::string Name );
	bool     AddProject( const std::filesystem::path& rPath );

//////////////////////////////////////////////////////////////////////////

	struct
	{
		Event< Workspace, void( std::filesystem::path OutputFile, bool Success ) > BuildFinished;

	} Events;

//////////////////////////////////////////////////////////////////////////

	BuildMatrix m_BuildMatrix;
	std::unique_ptr< Process > m_AppProcess;
}; // Workspace
