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

#include "Workspace.h"

#include "Compilers/CompilerGCC.h"

#include <iostream>

#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

Workspace::Workspace( std::filesystem::path location )
	: location_( std::move( location ) )
	, name_    ( "MyWorkspace" )
{
	compiler_ = std::make_unique< CompilerGCC >();
}

void Workspace::Build( void )
{
	Configuration cfg = build_matrix_.CurrentConfiguration();

	if( compiler_ && !projects_.empty() )
	{
		projects_left_to_build_.clear();

		// Keep track of which projects need to be built.
		for( Project& prj : projects_ )
			projects_left_to_build_.push_back( prj.name_ );

		BuildNextProject();
	}
}

bool Workspace::Serialize( void )
{
	if( location_.empty() )
		return false;

	GCL::Serializer serializer( ( location_ / name_ ).replace_extension( ext ) );
	if( !serializer.IsOpen() )
		return false;

	// Name string
	{
		GCL::Object name( "Name" );
		name.SetString( name_ );

		serializer.WriteObject( name );
	}

	// Matrix table
	{
		GCL::Object matrix( "Matrix", std::in_place_type< GCL::Object::TableType > );

		for( const BuildMatrix::Column& column : build_matrix_.columns_ )
		{
			SerializeBuildMatrixColumn( matrix, column );
		}

		serializer.WriteObject( matrix );
	}

	// Projects array
	{
		GCL::Object projects( "Projects", std::in_place_type< GCL::Object::TableType > );

		for( Project& prj : projects_ )
		{
			std::filesystem::path relative_project_path = prj.location_.lexically_relative( location_ ) / prj.name_;

			projects.AddChild( GCL::Object( relative_project_path.string() ) );

			prj.Serialize();
		}

		serializer.WriteObject( projects );
	}

	return true;
}

bool Workspace::Deserialize( void )
{
	if( location_.empty() )
		return false;

	GCL::Deserializer serializer( ( location_ / name_ ).replace_extension( ext ) );
	if( !serializer.IsOpen() )
		return false;

	serializer.Objects( this, GCLObjectCallback );

	return true;
}

Project& Workspace::NewProject( std::filesystem::path location, std::string name )
{
	Project& project = projects_.emplace_back( std::move( location ) );
	project.name_    = std::move( name );

	return project;
}

Project* Workspace::ProjectByName( std::string_view name )
{
	for( Project& prj : projects_ )
	{
		if( prj.name_ == name )
			return &prj;
	}

	return nullptr;
}

void Workspace::GCLObjectCallback( GCL::Object object, void* user )
{
	Workspace*       self = ( Workspace* )user;
	std::string_view name = object.Name();

	if( name == "Name" )
	{
		self->name_ = object.String();
	}
	else if( name == "Matrix" )
	{
		self->build_matrix_ = BuildMatrix();

		for( const GCL::Object& column : object.Table() )
		{
			std::string_view column_name = column.Name();

			self->build_matrix_.NewColumn( std::string( column_name ) );
			self->DeserializeBuildMatrixColumn( self->build_matrix_.columns_.back(), column );
		}
	}
	else if( name == "Projects" )
	{
		for( auto& prj_path_string : object.Table() )
		{
			std::filesystem::path project_path = prj_path_string.String();

			if( !project_path.is_absolute() )
				project_path = self->location_ / project_path;

			project_path = project_path.lexically_normal();

			Project& project = self->NewProject( project_path.parent_path(), project_path.filename().string() );
			project.Deserialize();
		}
	}
}

void Workspace::BuildNextProject( void )
{
	if( projects_left_to_build_.empty() )
		return;

	// Find the next project to build
	auto it = std::find_if( projects_.begin(), projects_.end(),
		[ this ]( Project& prj )
		{
			return ( prj.name_ == projects_left_to_build_.back() );
		}
	);

	if( it == projects_.end() )
	{
		// If next project was not found, remove it from the queue and try again
		projects_left_to_build_.pop_back();
		BuildNextProject();
	}
	else
	{
		std::cout << "=== Started building " << it->name_ << " ===\n";

		*it ^= [ this ]( const ProjectBuildFinished& e )
		{
			if( e.success ) std::cout << "=== " << e.project->name_ << " finished successfully ===\n";
			else            std::cerr << "=== " << e.project->name_ << " finished with errors ===\n";

			auto it = std::find( projects_left_to_build_.begin(), projects_left_to_build_.end(), e.project->name_ );
			if( it != projects_left_to_build_.end() )
			{
				projects_left_to_build_.erase( it );

				if( projects_left_to_build_.empty() ) OnBuildFinished( e.output, e.success );
				else                                  BuildNextProject();
			}
			else
			{
				std::cerr << "Project was preemptively popped from list\n";
			}
		};

		it->Build( *compiler_ );
	}
}

void Workspace::OnBuildFinished( const std::filesystem::path& output, bool success )
{
	WorkspaceBuildFinished e;
	e.workspace = this;
	e.output    = output;
	e.success   = success;
	Publish( e );
}

void Workspace::SerializeBuildMatrixColumn( GCL::Object& object, const BuildMatrix::Column& column )
{
	GCL::Object column_object( column.name, std::in_place_type< GCL::Object::TableType > );

	for( auto& cfg : column.configurations )
	{
		GCL::Object cfg_object( cfg.name );

		if( !cfg.exclusive_columns.empty() )
		{
			cfg_object.SetTable();

			for( auto& exclusive : cfg.exclusive_columns )
			{
				SerializeBuildMatrixColumn( cfg_object, exclusive );
			}
		}

		column_object.AddChild( std::move( cfg_object ) );
	}

	object.AddChild( std::move( column_object ) );
}

void Workspace::DeserializeBuildMatrixColumn( BuildMatrix::Column& column, const GCL::Object& object )
{
	for( auto& cfg : object.Table() )
	{
		BuildMatrix::NamedConfiguration new_cfg;
		new_cfg.name = cfg.Name();

		if( cfg.IsTable() )
		{
			for( auto& exclusive : cfg.Table() )
			{
				BuildMatrix::Column exclusive_column;
				exclusive_column.name = exclusive.Name();

				DeserializeBuildMatrixColumn( exclusive_column, exclusive );

				new_cfg.exclusive_columns.emplace_back( std::move( exclusive_column ) );
			}
		}

		column.configurations.emplace_back( std::move( new_cfg ) );
	}
}
