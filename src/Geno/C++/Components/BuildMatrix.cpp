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

#include "Compilers/CompilerGCC.h"
#include "Compilers/CompilerMSVC.h"

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
			column.Configurations.emplace_back( std::move( Configuration ), ::Configuration() );
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
		auto CurrentConfiguration = std::find_if( rColumn.Configurations.begin(), rColumn.Configurations.end(), [ &rColumn ]( const auto& rPair ) { return rPair.first == rColumn.CurrentConfiguration; } );
		if( CurrentConfiguration == rColumn.Configurations.end() )
			continue;

		Result.Override( CurrentConfiguration->second );
	}

	return Result;

} // CurrentConfiguration

//////////////////////////////////////////////////////////////////////////

BuildMatrix BuildMatrix::PlatformDefault( void )
{
	BuildMatrix Matrix;

	// Target
	{
		Column Target;
		Target.Name = "Target";

	#if defined( _WIN32 )
		{
			Configuration WindowsConfiguration;
			WindowsConfiguration.m_Compiler = std::make_shared< CompilerMSVC >();
			Target.Configurations.emplace_back( "Windows", std::move( WindowsConfiguration ) );
		}
	#elif defined( __linux__ ) // _WIN32
		{
			Configuration LinuxConfiguration;
			LinuxConfiguration.m_Compiler = std::make_shared< CompilerGCC >();
			Target.Configurations.emplace_back( "Linux", std::move( LinuxConfiguration ) );
		}
	#elif defined( __APPLE__ ) // __linux__
		{
			Configuration MacOSConfiguration;
			MacOSConfiguration.m_Compiler = nullptr;
			Target.Configurations.emplace_back( "macOS", std::move( MacOSConfiguration ) );
		}
	#endif // __APPLE__

		Matrix.m_Columns.emplace_back( std::move( Target ) );
	}

	// Architecture
	{
		Column Platform;
		Platform.Name = "Architecture";
		Platform.Configurations.emplace_back( "x86",    Configuration() ).second.m_Architecture = Configuration::Architecture::x86;
		Platform.Configurations.emplace_back( "x86_64", Configuration() ).second.m_Architecture = Configuration::Architecture::x86_64;
		Platform.Configurations.emplace_back( "ARM",    Configuration() ).second.m_Architecture = Configuration::Architecture::ARM;
		Platform.Configurations.emplace_back( "ARM64",  Configuration() ).second.m_Architecture = Configuration::Architecture::ARM64;
		Matrix.m_Columns.emplace_back( std::move( Platform ) );
	}

	// Optimization
	{
		Column Optimization;
		Optimization.Name = "Optimization";
		Optimization.Configurations.emplace_back( "Off",         Configuration() );
		Optimization.Configurations.emplace_back( "Favor Size",  Configuration() ).second.m_Optimization = Configuration::Optimization::FavorSize;
		Optimization.Configurations.emplace_back( "Favor Speed", Configuration() ).second.m_Optimization = Configuration::Optimization::FavorSpeed;
		Optimization.Configurations.emplace_back( "Full",        Configuration() ).second.m_Optimization = Configuration::Optimization::Full;
		Matrix.m_Columns.emplace_back( std::move( Optimization ) );
	}

	return Matrix;

} // PlatformDefault
