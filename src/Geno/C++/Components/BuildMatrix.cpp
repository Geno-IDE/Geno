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

#include "BuildMatrix.h"

#include <Common/Intrinsics.h>

void BuildMatrix::NewColumn( std::string name )
{
	Column column;
	column.name = std::move( name );

	columns_.emplace_back( std::move( column ) );
}

void BuildMatrix::NewConfiguration( std::string_view which_column, std::string configuration )
{
	for( Column& column : columns_ )
	{
		if( column.name == which_column )
		{
			column.configurations.push_back( { std::move( configuration ) } );
			return;
		}
	}
}

Configuration BuildMatrix::CurrentConfiguration( void ) const
{
	Configuration result;

	// Combine the values of all current configurations in each column
	for( const Column& column : columns_ )
	{
		if( column.current_configuration.empty() )
			continue;

		// Find the current configuration
		auto it = std::find_if( column.configurations.begin(), column.configurations.end(),
			[ &column ]( const NamedConfiguration& cfg )
			{
				return cfg.name == column.current_configuration;
			}
		);

		if( it == column.configurations.end() )
			continue;

		result.CombineWith( it->configuration );
	}

	return result;
}

BuildMatrix BuildMatrix::PlatformDefault( void )
{
	BuildMatrix matrix;

	Column platform_column;
	platform_column.name = "Platform";
	platform_column.configurations.push_back( { ( std::string )Intrinsics::TargetMachine() } );
	matrix.columns_.emplace_back( std::move( platform_column ) );

	Column optimization_column;
	optimization_column.name = "Optimization";
	optimization_column.configurations.push_back( { "Full" } );
	optimization_column.configurations.push_back( { "Favor Size" } );
	optimization_column.configurations.push_back( { "Favor Speed" } );
	optimization_column.configurations.push_back( { "Off" } );
	matrix.columns_.emplace_back( std::move( optimization_column ) );

	Column symbols_column;
	symbols_column.name = "Symbols";
	symbols_column.configurations.push_back( { "On" } );
	symbols_column.configurations.push_back( { "Off" } );
	matrix.columns_.emplace_back( std::move( symbols_column ) );

	return matrix;
}
