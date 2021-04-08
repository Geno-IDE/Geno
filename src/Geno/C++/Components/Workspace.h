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
#include "Components/Project.h"

#include <Common/EventDispatcher.h>
#include <GCL/Deserializer.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class Workspace;

struct WorkspaceBuildFinished
{
	Workspace*            pWorkspace;
	std::filesystem::path Output;
	bool                  Success = false;

}; // WorkspaceBuildFinished

class Workspace : public EventDispatcher< Workspace, WorkspaceBuildFinished >
{
	GENO_DISABLE_COPY( Workspace );
	GENO_DEFAULT_MOVE( Workspace );

//////////////////////////////////////////////////////////////////////////

public:

	static constexpr std::string_view EXTENSION = ".gwks";

//////////////////////////////////////////////////////////////////////////

	explicit Workspace( std::filesystem::path Location );

//////////////////////////////////////////////////////////////////////////

	void Build      ( void );
	bool Serialize  ( void );
	bool Deserialize( void );

//////////////////////////////////////////////////////////////////////////

	Project& NewProject   ( std::filesystem::path Location, std::string Name );
	Project* ProjectByName( std::string_view Name );

//////////////////////////////////////////////////////////////////////////

	BuildMatrix                  m_BuildMatrix;
	std::filesystem::path        m_Location;
	std::string                  m_Name;
	std::vector< Project >       m_Projects;
	std::unique_ptr< ICompiler > m_Compiler;
	std::vector< std::string >   m_ProjectsLeftToBuild;

//////////////////////////////////////////////////////////////////////////

private:

	static void GCLObjectCallback( GCL::Object pObject, void* pUser );

//////////////////////////////////////////////////////////////////////////

	void BuildNextProject            ( void );
	void OnBuildFinished             ( const std::filesystem::path& rOutput, bool Success );
	void SerializeBuildMatrixColumn  ( GCL::Object& rObject, const BuildMatrix::Column& rColumn );
	void DeserializeBuildMatrixColumn( BuildMatrix::Column& rColumn, const GCL::Object& rObject );

}; // Workspace
