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

void Workspace::Build( void )
{
	for( Project& prj : projects_ )
	{
		prj.Build();
	}
}

void Workspace::Serialize( void )
{
	if( !location_.empty() )
	{
		GCL::Serializer serializer( location_ );
	}
}

void Workspace::Deserialize( void )
{
	if( !location_.empty() )
	{
		GCL::Deserializer serializer( location_, GCLObjectCallback, this );
	}
}

std::filesystem::path Workspace::RelativePath( const std::filesystem::path& path ) const
{
	if( path.is_absolute() )
		return ( path.lexically_relative( location_.parent_path() ) );
	else
		return ( location_.parent_path() / path );
}

void Workspace::GCLObjectCallback( GCL::Object object, void* user )
{
	Workspace* self = ( Workspace* )user;

	if( object.Key() == "Name" )
	{
		self->name_ = object;

		std::cout << "Workspace: " << self->name_ << "\n";
	}
	else if( object.Key() == "Matrix" )
	{
		self->build_matrix_ = BuildMatrix();

		for( const GCL::Object& column : object.Children() )
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
				prj_path = self->RelativePath( prj_path );

			if( !prj_path.has_extension() )
				prj_path += ".gprj";

			prj_path.make_preferred();

			Project prj;
			prj.SetLocation( prj_path );
			prj.Deserialize();

			self->projects_.emplace_back( std::move( prj ) );
		}
	}
}
