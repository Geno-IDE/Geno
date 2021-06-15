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

#include "Workspace.h"

#include "Compilers/CompilerGCC.h"
#include "Compilers/CompilerMSVC.h"

#include <iostream>

#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

//////////////////////////////////////////////////////////////////////////

Workspace::Workspace( std::filesystem::path Location )
	: m_Location( std::move( Location ) )
	, m_Name    ( "MyWorkspace" )
{

#if defined( _WIN32 )
	m_Compiler = std::make_unique< CompilerMSVC >();
#else // _WIN32
	m_Compiler = std::make_unique< CompilerGCC >();
#endif // !_WIN32

} // Workspace

//////////////////////////////////////////////////////////////////////////

void Workspace::Build( void )
{
	Configuration Configuration = m_BuildMatrix.CurrentConfiguration();

	if( m_Compiler && !m_Projects.empty() )
	{
		m_ProjectsLeftToBuild.clear();

		// Keep track of which projects need to be built.
		for( Project& prj : m_Projects )
			m_ProjectsLeftToBuild.push_back( prj.m_Name );

		BuildNextProject();
	}

} // Build

//////////////////////////////////////////////////////////////////////////

bool Workspace::Serialize( void )
{
	if( m_Location.empty() )
		return false;

	GCL::Serializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );
	if( !Serializer.IsOpen() )
		return false;

	// Name string
	{
		GCL::Object Name( "Name" );
		Name.SetString( m_Name );

		Serializer.WriteObject( Name );
	}

	// Matrix table
	{
		GCL::Object Matrix( "Matrix", std::in_place_type< GCL::Object::TableType > );

		for( const BuildMatrix::Column& column : m_BuildMatrix.m_Columns )
		{
			SerializeBuildMatrixColumn( Matrix, column );
		}

		Serializer.WriteObject( Matrix );
	}

	// Projects array
	{
		GCL::Object Projects( "Projects", std::in_place_type< GCL::Object::TableType > );

		for( Project& rProject : m_Projects )
		{
			std::filesystem::path relative_project_path = rProject.m_Location.lexically_relative( m_Location ) / rProject.m_Name;

			Projects.AddChild( GCL::Object( relative_project_path.string() ) );

			rProject.Serialize();
		}

		Serializer.WriteObject( Projects );
	}

	return true;

} // Serialize

//////////////////////////////////////////////////////////////////////////

bool Workspace::Deserialize( void )
{
	if( m_Location.empty() )
		return false;

	GCL::Deserializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );
	if( !Serializer.IsOpen() )
		return false;

	Serializer.Objects( this, GCLObjectCallback );

	return true;

} // Deserialize

//////////////////////////////////////////////////////////////////////////

Project& Workspace::NewProject( std::filesystem::path Location, std::string Name )
{
	Project& project = m_Projects.emplace_back( std::move( Location ) );
	project.m_Name    = std::move( Name );

	return project;

} // NewProject

//////////////////////////////////////////////////////////////////////////

Project* Workspace::ProjectByName( std::string_view Name )
{
	for( Project& rProject : m_Projects )
	{
		if( rProject.m_Name == Name )
			return &rProject;
	}

	return nullptr;

} // ProjectByName

//////////////////////////////////////////////////////////////////////////

void Workspace::GCLObjectCallback( GCL::Object pObject, void* pUser )
{
	Workspace*       pSelf = ( Workspace* )pUser;
	std::string_view Name  = pObject.Name();

	if( Name == "Name" )
	{
		pSelf->m_Name = pObject.String();
	}
	else if( Name == "Matrix" )
	{
		pSelf->m_BuildMatrix = BuildMatrix();

		for( const GCL::Object& rColumn : pObject.Table() )
		{
			const std::string_view ColumnName = rColumn.Name();

			pSelf->m_BuildMatrix.NewColumn( std::string( ColumnName ) );
			pSelf->DeserializeBuildMatrixColumn( pSelf->m_BuildMatrix.m_Columns.back(), rColumn );
		}
	}
	else if( Name == "Projects" )
	{
		for( const GCL::Object& rProjectPathObj : pObject.Table() )
		{
			std::filesystem::path ProjectPath = rProjectPathObj.String();

			if( !ProjectPath.is_absolute() )
				ProjectPath = pSelf->m_Location / ProjectPath;

			ProjectPath = ProjectPath.lexically_normal();

			Project& rProject = pSelf->NewProject( ProjectPath.parent_path(), ProjectPath.filename().string() );
			rProject.Deserialize();
		}
	}

} // GCLObjectCallback

//////////////////////////////////////////////////////////////////////////

void Workspace::BuildNextProject( void )
{
	if( m_ProjectsLeftToBuild.empty() )
		return;

	// Find the next project to build
	auto ProjectIt = std::find_if( m_Projects.begin(), m_Projects.end(), [ this ]( ::Project& rProject ) { return ( rProject.m_Name == m_ProjectsLeftToBuild.back() ); } );
	if( ProjectIt == m_Projects.end() )
	{
		// If next project was not found, remove it from the queue and try again
		m_ProjectsLeftToBuild.pop_back();
		BuildNextProject();
	}
	else
	{
		std::cout << "=== Started building " << ProjectIt->m_Name << " ===\n";

		ProjectIt->Events.BuildFinished += [ this ]( Project& rProject, std::filesystem::path OutputFile, bool Success )
		{
			if( Success ) std::cout << "=== " << rProject.m_Name << " finished successfully ===\n";
			else          std::cerr << "=== " << rProject.m_Name << " finished with errors ===\n";

			auto NextProject = std::find( m_ProjectsLeftToBuild.begin(), m_ProjectsLeftToBuild.end(), rProject.m_Name );
			if( NextProject != m_ProjectsLeftToBuild.end() )
			{
				m_ProjectsLeftToBuild.erase( NextProject );

				if( m_ProjectsLeftToBuild.empty() ) OnBuildFinished( OutputFile, Success );
				else                                BuildNextProject();
			}
			else
			{
				std::cerr << "Project was preemptively popped from list\n";
			}
		};

		ProjectIt->Build( *m_Compiler );
	}

} // BuildNextProject

//////////////////////////////////////////////////////////////////////////

void Workspace::OnBuildFinished( const std::filesystem::path& rOutput, bool Success )
{
	Events.BuildFinished( *this, rOutput, Success );

} // OnBuildFinished

//////////////////////////////////////////////////////////////////////////

void Workspace::SerializeBuildMatrixColumn( GCL::Object& rObject, const BuildMatrix::Column& rColumn )
{
	GCL::Object ColumnObj( rColumn.Name, std::in_place_type< GCL::Object::TableType > );

	for( const BuildMatrix::NamedConfiguration& rConfiguration : rColumn.Configurations )
	{
		GCL::Object ConfigurationObj( rConfiguration.Name );

		if( !rConfiguration.ExclusiveColumns.empty() )
		{
			ConfigurationObj.SetTable();

			for( const BuildMatrix::Column& rExclusiveColumn : rConfiguration.ExclusiveColumns )
			{
				SerializeBuildMatrixColumn( ConfigurationObj, rExclusiveColumn );
			}
		}

		ColumnObj.AddChild( std::move( ConfigurationObj ) );
	}

	rObject.AddChild( std::move( ColumnObj ) );

} // SerializeBuildMatrixColumn

//////////////////////////////////////////////////////////////////////////

void Workspace::DeserializeBuildMatrixColumn( BuildMatrix::Column& rColumn, const GCL::Object& rObject )
{
	for( const GCL::Object& rConfigurationObj : rObject.Table() )
	{
		BuildMatrix::NamedConfiguration NewConfiguration;
		NewConfiguration.Name = rConfigurationObj.Name();

		if( rConfigurationObj.IsTable() )
		{
			for( const GCL::Object& rExclusiveColumnObj : rConfigurationObj.Table() )
			{
				BuildMatrix::Column ExclusiveColumn;
				ExclusiveColumn.Name = rExclusiveColumnObj.Name();

				DeserializeBuildMatrixColumn( ExclusiveColumn, rExclusiveColumnObj );

				NewConfiguration.ExclusiveColumns.emplace_back( std::move( ExclusiveColumn ) );
			}
		}

		rColumn.Configurations.emplace_back( std::move( NewConfiguration ) );
	}

} // DeserializeBuildMatrixColumn
