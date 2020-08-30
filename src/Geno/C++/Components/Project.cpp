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

Project::Project( std::filesystem::path location )
	: location_( std::move( location ) )
	, name_    ( "MyProject" )
{
}

Project::Project( Project&& other )
{
	*this = std::move( other );
}

Project& Project::operator=( Project&& other )
{
	kind_                = other.kind_; other.kind_ = ProjectKind::Unknown;
	location_            = std::move( other.location_ );
	name_                = std::move( other.name_ );
	files_               = std::move( other.files_ );
	includes_            = std::move( other.includes_ );
	libraries_           = std::move( other.libraries_ );
	configrations_       = std::move( other.configrations_ );
	files_left_to_build_ = std::move( other.files_left_to_build_ );
	files_to_link_       = std::move( other.files_to_link_ );

	return *this;
}

void Project::Build( ICompiler& compiler )
{
	files_left_to_build_.clear();
	files_to_link_.clear();

	if( files_.empty() )
		return;

	for( const std::filesystem::path& cpp : files_ )
	{
		// #TODO: Compiler will be per-file so this check is only temporary
		if( cpp.extension() != ".cpp" )
			continue;

		// Keep track of which files are currently building
		files_left_to_build_.push_back( cpp );
	}

	BuildNextFile( compiler );
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

	// Kind
	{
		GCL::Object kind( "Kind" );
		kind.SetString( ProjectKindToString( kind_ ) );
		serializer.WriteObject( kind );
	}

	// Files
	{
		GCL::Object files( "Files", std::in_place_type< GCL::Object::TableType > );

		for( const std::filesystem::path& file : files_ )
		{
			const std::filesystem::path relative_path = file.lexically_relative( location_ );

			files.AddChild( GCL::Object( relative_path.string() ) );
		}

		serializer.WriteObject( files );
	}

	// Libraries
	{
		GCL::Object libraries( "Libraries", std::in_place_type< GCL::Object::TableType > );

		for( const std::filesystem::path& library : libraries_ )
		{
			const std::filesystem::path relative_path = library.lexically_relative( location_ );

			libraries.AddChild( GCL::Object( relative_path.string() ) );
		}

		serializer.WriteObject( libraries );
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
	Project*         self = static_cast< Project* >( user );
	std::string_view name = object.Name();

	if( name == "Name" )
	{
		self->name_ = object.String();
	}
	else if( name == "Kind" )
	{
		self->kind_ = ProjectKindFromString( object.String() );
	}
	else if( name == "Files" )
	{
		for( auto& file_path_string : object.Table() )
		{
			std::filesystem::path file_path = file_path_string.String();

			if( !file_path.is_absolute() )
				file_path = self->location_ / file_path;

			file_path = file_path.lexically_normal();
			self->files_.emplace_back( std::move( file_path ) );
		}
	}
	else if( name == "Includes" )
	{
		for( auto& file_path_string : object.Table() )
		{
			std::filesystem::path file_path = file_path_string.String();

			if( !file_path.is_absolute() )
				file_path = self->location_ / file_path;

			file_path = file_path.lexically_normal();
			self->includes_.emplace_back( std::move( file_path ) );
		}
	}
	else if( name == "Libraries" )
	{
		for( auto& library : object.Table() )
		{
			std::filesystem::path path = library.String();

			if( !path.is_absolute() )
				path = self->location_ / path;

			path = path.lexically_normal();
			self->libraries_.emplace_back( std::move( path ) );
		}
	}
}

void Project::BuildNextFile( ICompiler& compiler )
{
	if( files_left_to_build_.empty() )
	{
		Link( compiler );

		return;
	}

//////////////////////////////////////////////////////////////////////////

	auto it = std::find( files_.begin(), files_.end(), files_left_to_build_.back() );

	if( it == files_.end() )
	{
		// If the file was not found, remove it from the queue and try again
		files_left_to_build_.pop_back();
		BuildNextFile( compiler );
	}
	else
	{
		// Listen to every file compilation to check if we're 
		compiler ^= [ this, &compiler ]( const CompilationDone& e )
		{
			// Cancel build if a file failed to build
			if( e.exit_code != 0 )
			{
				ProjectBuildFinished build_finished;
				build_finished.project = this;
				build_finished.success = false;

				Publish( build_finished );
			}
			else if( auto it = std::find( files_left_to_build_.begin(), files_left_to_build_.end(), e.options.input_file ); it != files_left_to_build_.end() )
			{
				files_to_link_.push_back( e.options.output_file );
				files_left_to_build_.erase( it );
				BuildNextFile( compiler );
			}
		};

		CompileOptions options;
		options.input_file  = *it;
		options.output_file = location_ / it->filename();
		options.output_file.replace_extension( ".o" );
		options.language    = CompileOptions::Language::CPlusPlus;
		options.action      = CompileOptions::Action::CompileAndAssemble;

		// Compile the file
		compiler.Compile( options );
	}
}

void Project::Link( ICompiler& compiler )
{
	compiler ^= [ this ]( const LinkingDone& e )
	{
		ProjectBuildFinished build_finished;
		build_finished.project = this;
		build_finished.success = e.exit_code == 0;

		Publish( build_finished );
	};

	LinkOptions options;
	options.input_files      = std::move( files_to_link_ );
	options.linked_libraries = libraries_;
	options.output_file      = location_ / name_;
	options.output_file.replace_extension( ProjectKindOutputExtension( kind_ ) );
	options.kind             = kind_;

	compiler.Link( options );
}
