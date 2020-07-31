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
		GCL::Deserializer serializer( location_, GCLValueCallback, this );
	}
}

void Workspace::GCLValueCallback( const GCL::Deserializer::KeyedValues& values, void* user )
{
	Workspace*       self = ( Workspace* )user;
	std::string_view key  = values.key_or_value;

	if( key == "Name" )
	{
		self->name_ = values.values.begin()->key_or_value;

		std::cout << "Loaded workspace: " << self->name_ << "\n";
	}
	else if( key == "Matrix" )
	{
		self->matrix_ = BuildMatrix();

		for( auto& value : values.values )
		{
			self->matrix_.AddColumn( value.key_or_value );

			std::string_view configurations = value.values.begin()->key_or_value;
			for( size_t cfg_begin = 0, cfg_end = 0; cfg_begin != std::string_view::npos; )
			{
				cfg_end = configurations.find( ',', cfg_begin );

				if( cfg_end == std::string_view::npos )
				{
					std::string_view cfg = configurations.substr( cfg_begin );
					self->matrix_.AddConfiguration( value.key_or_value, cfg );
					cfg_begin = std::string_view::npos;
					std::cout << "Added to matrix: " << value.key_or_value << "|" << cfg << "\n";
				}
				else
				{
					std::string_view cfg = configurations.substr( cfg_begin, cfg_end - cfg_begin );
					self->matrix_.AddConfiguration( value.key_or_value, cfg );
					cfg_begin = cfg_end + 1;
					std::cout << "Added to matrix: " << value.key_or_value << "|" << cfg << "\n";
				}
			}

		}
	}
}
