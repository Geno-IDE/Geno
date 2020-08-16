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

#include "Project.h"

#include "Compilers/ICompiler.h"

#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

#include <iostream>

Project::Project( const std::filesystem::path& location )
	: location_( location )
	, name_    ( "MyProject" )
{
}

void Project::Build( ICompiler& compiler )
{
	for( const std::filesystem::path& cpp : files_ )
	{
		compiler.Compile( cpp );
	}
}

bool Project::Serialize( void )
{
	if( location_.empty() )
	{
		std::cerr << "Failed to serialize ";

		if( name_.empty() ) std::cerr << "unnamed project.";
		else                std::cerr << "project '" << name_ << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	GCL::Serializer serializer( ( location_ / name_ ).replace_extension( ext ) );
	if( !serializer.IsOpen() )
		return false;

	// Name
	{
		GCL::Object name( "Name" );
		name.SetString( name_ );
		serializer.WriteObject( name );
	}

	// Files
	{
		GCL::Object              files( "Files", std::in_place_type< GCL::Object::ArrayType > );
		std::list< std::string > relative_file_path_strings;
		for( const std::filesystem::path& file : files_ )
		{
			std::filesystem::path relative_file_path        = file.lexically_relative( location_ );
			std::string&          relative_file_path_string = relative_file_path_strings.emplace_back( relative_file_path.string() );

			files.AddArrayItem( relative_file_path_string );
		}

		serializer.WriteObject( files );
	}

	return true;
}

bool Project::Deserialize( void )
{
	if( location_.empty() )
	{
		std::cerr << "Failed to deserialize ";

		if( name_.empty() ) std::cerr << "unnamed project.";
		else                std::cerr << "project '" << name_ << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	GCL::Deserializer deserializer( ( location_ / name_ ).replace_extension( ext ) );
	if( !deserializer.IsOpen() )
		return false;

	deserializer.Objects( this, GCLObjectCallback );

	return true;
}

void Project::GCLObjectCallback( GCL::Object object, void* user )
{
	Project* self = static_cast< Project* >( user );

	if( object.Key() == "Name" )
	{
		self->name_ = object.String();

		std::cout << "Project: " << self->name_ << "\n";
	}
	else if( object.Key() == "Files" )
	{
		for( std::string_view file_path_string : object.Array() )
		{
			std::filesystem::path file_path = file_path_string;

			if( !file_path.is_absolute() )
				file_path = self->location_ / file_path;

			file_path = file_path.lexically_normal();
			self->files_.emplace_back( std::move( file_path ) );
		}
	}
	else if( object.Key() == "Includes" )
	{
		for( std::string_view file_path_string : object.Array() )
		{
			std::filesystem::path file_path = file_path_string;

			if( !file_path.is_absolute() )
				file_path = self->location_ / file_path;

			file_path = file_path.lexically_normal();
			self->includes_.emplace_back( std::move( file_path ) );
		}
	}
}
