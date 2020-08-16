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

#include <iostream>

#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

Workspace::Workspace( const std::filesystem::path& location )
	: location_( location )
	, name_    ( "MyWorkspace" )
{
}

void Workspace::Build( void )
{
	for( Project& prj : projects_ )
	{
		prj.Build();
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
		for( BuildMatrix::Column& column : build_matrix_.columns_ )
		{
			GCL::Object child( column.name, std::in_place_type< GCL::Object::ArrayType > );

			for( const std::string& cfg : column.configurations )
				child.AddArrayItem( cfg );

			matrix.AddChild( std::move( child ) );
		}

		serializer.WriteObject( matrix );
	}

	// Projects array
	{
		GCL::Object              projects( "Projects", std::in_place_type< GCL::Object::ArrayType > );
		std::list< std::string > relative_project_path_strings; // GCL Arrays store its elements as std::string_view which means the string needs to live until we call WriteObject
		for( Project& prj : projects_ )
		{
			std::filesystem::path relative_project_path        = prj.location_.lexically_relative( location_ ) / prj.name_;
			std::string&          relative_project_path_string = relative_project_path_strings.emplace_back( relative_project_path.string() );

			projects.AddArrayItem( relative_project_path_string );

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
	Workspace* self = ( Workspace* )user;

	if( object.Key() == "Name" )
	{
		self->name_ = object.String();

		std::cout << "Workspace: " << self->name_ << "\n";
	}
	else if( object.Key() == "Matrix" )
	{
		self->build_matrix_ = BuildMatrix();

		for( const GCL::Object& column : object.Table() )
		{
			self->build_matrix_.AddColumn( column.Key() );

			for( std::string_view cfg : column.Array() )
			{
				self->build_matrix_.AddConfiguration( column.Key(), cfg );

				std::cout << "Configuration: " << column.Key() << "|" << cfg << "\n";
			}
		}
	}
	else if( object.Key() == "Projects" )
	{
		for( std::string_view prj_path_string : object.Array() )
		{
			std::filesystem::path prj_path = prj_path_string;

			if( !prj_path.is_absolute() )
				prj_path = self->location_ / prj_path;

			prj_path = prj_path.lexically_normal();

			Project prj( prj_path.parent_path() );
			prj.name_ = prj_path.filename().string();

			if( prj.Deserialize() )
				self->projects_.emplace_back( std::move( prj ) );
		}
	}
}
