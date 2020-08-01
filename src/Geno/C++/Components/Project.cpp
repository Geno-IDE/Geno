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

#include <GCL/Deserializer.h>

#include <iostream>

void Project::Deserialize( void )
{
	if( !location_.empty() )
	{
		GCL::Deserializer deserializer( location_, GCLObjectCallback, this );
	}
}

void Project::GCLObjectCallback( GCL::Object object, void* user )
{
	Project* self = static_cast< Project* >( user );

	if( object.Key() == "Name" )
	{
		self->name_ = object;

		std::cout << "Project: " << self->name_ << "\n";
	}
	else if( object.Key() == "Files" )
	{
		for( std::string_view file_path_string : object.Array() )
		{
			std::filesystem::path file_path = file_path_string;

			if( !file_path.is_absolute() )
				file_path = self->location_.parent_path() / file_path;

			file_path.make_preferred();
			self->files_.emplace_back( std::move( file_path ) );
		}
	}
	else if( object.Key() == "Includes" )
	{
		for( std::string_view file_path_string : object.Array() )
		{
			std::filesystem::path file_path = file_path_string;

			if( !file_path.is_absolute() )
				file_path = self->location_.parent_path() / file_path;

			file_path.make_preferred();
			self->includes_.emplace_back( std::move( file_path ) );
		}
	}
}
