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

#include "BuildMatrix.h"

#include <Common/Intrinsics.h>

#include <algorithm>

//////////////////////////////////////////////////////////////////////////

void BuildMatrix::NewColumn( std::string Name )
{
	Column Column;
	Column.Name = std::move( Name );

	m_Columns.emplace_back( std::move( Column ) );

} // NewColumn

//////////////////////////////////////////////////////////////////////////

void BuildMatrix::NewConfiguration( std::string_view WhichColumn, std::string Configuration )
{
	for( Column& column : m_Columns )
	{
		if( column.Name == WhichColumn )
		{
			column.Configurations.try_emplace( std::move( Configuration ) );
			return;
		}
	}

} // NewConfiguration

//////////////////////////////////////////////////////////////////////////

Configuration BuildMatrix::CurrentConfiguration( void ) const
{
	Configuration Result;

	// Combine the values of all current configurations in each column
	for( const Column& rColumn : m_Columns )
	{
		if( rColumn.CurrentConfiguration.empty() )
			continue;

		// Find the current configuration
		auto CurrentConfiguration = rColumn.Configurations.find( rColumn.CurrentConfiguration );
		if( CurrentConfiguration == rColumn.Configurations.end() )
			continue;

		Result.CombineWith( CurrentConfiguration->second );
	}

	return Result;

} // CurrentConfiguration

//////////////////////////////////////////////////////////////////////////

BuildMatrix BuildMatrix::PlatformDefault( void )
{
	BuildMatrix Matrix;

	Column PlatformColumn;
	PlatformColumn.Name = "Platform";
	PlatformColumn.Configurations.try_emplace( std::string( Intrinsics::TargetMachine() ) );
	Matrix.m_Columns.emplace_back( std::move( PlatformColumn ) );

	Column OptimizationColumn;
	OptimizationColumn.Name = "Optimization";
	OptimizationColumn.Configurations.try_emplace( "Full" );
	OptimizationColumn.Configurations.try_emplace( "Favor Size" );
	OptimizationColumn.Configurations.try_emplace( "Favor Speed" );
	OptimizationColumn.Configurations.try_emplace( "Off" );
	Matrix.m_Columns.emplace_back( std::move( OptimizationColumn ) );

	Column SymbolsColumn;
	SymbolsColumn.Name = "Debug Symbols";
	SymbolsColumn.Configurations.try_emplace( "On" );
	SymbolsColumn.Configurations.try_emplace( "Off" );
	Matrix.m_Columns.emplace_back( std::move( SymbolsColumn ) );

	return Matrix;

} // PlatformDefault
