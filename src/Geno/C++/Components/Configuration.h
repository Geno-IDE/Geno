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

#pragma once
#include <Common/Macros.h>

#include <filesystem>
#include <optional>
#include <memory>

class ICompiler;

//////////////////////////////////////////////////////////////////////////

class Configuration
{
	GENO_DEFAULT_COPY( Configuration );
	GENO_DEFAULT_MOVE( Configuration );

//////////////////////////////////////////////////////////////////////////

public:

	enum class Optimization
	{
		FavorSize,
		FavorSpeed,
		Full,

	}; // Optimization

	enum class Architecture
	{
		x86,
		x86_64,
		ARM,
		ARM64,

	}; // Architecture

//////////////////////////////////////////////////////////////////////////

	Configuration( void ) = default;

//////////////////////////////////////////////////////////////////////////

	void Override( const Configuration& rOther );

//////////////////////////////////////////////////////////////////////////

	static Architecture HostArchitecture( void );

//////////////////////////////////////////////////////////////////////////

	std::shared_ptr< ICompiler >         m_Compiler;
	std::vector< std::filesystem::path > m_IncludeDirs;
	std::vector< std::filesystem::path > m_LibraryDirs;
	std::vector< std::wstring >          m_Libraries;
	std::vector< std::wstring >          m_Defines;
	std::optional< Optimization >        m_Optimization;
	std::optional< Architecture >        m_Architecture;
	std::filesystem::path                m_OutputDir;
	bool                                 m_Verbose = true;

}; // Configuration

//////////////////////////////////////////////////////////////////////////

namespace Reflection
{
	constexpr std::string_view EnumToString( Configuration::Architecture Value )
	{
		switch( Value )
		{
			case Configuration::Architecture::x86:    return "x86";
			case Configuration::Architecture::x86_64: return "x86_64";
			case Configuration::Architecture::ARM:    return "ARM";
			case Configuration::Architecture::ARM64:  return "ARM64";
			default:                                  return "Unknown";
		}

	} // EnumToString

//////////////////////////////////////////////////////////////////////////

	constexpr std::string_view EnumToString( Configuration::Optimization Value )
	{
		switch( Value )
		{
			case Configuration::Optimization::FavorSize:  return "FavorSize";
			case Configuration::Optimization::FavorSpeed: return "FavorSpeed";
			case Configuration::Optimization::Full:       return "Full";
			default:                                      return "Unknown";
		}

	} // EnumToString

//////////////////////////////////////////////////////////////////////////

	constexpr void EnumFromString( std::string_view String, Configuration::Architecture& rValue )
	{
		if(      String == "x86"    ) rValue = Configuration::Architecture::x86;
		else if( String == "x86_64" ) rValue = Configuration::Architecture::x86_64;
		else if( String == "ARM"    ) rValue = Configuration::Architecture::ARM;
		else if( String == "ARM64"  ) rValue = Configuration::Architecture::ARM64;

	} // EnumFromString

//////////////////////////////////////////////////////////////////////////

	constexpr void EnumFromString( std::string_view String, Configuration::Optimization& rValue )
	{
		if(      String == "FavorSize"  ) rValue = Configuration::Optimization::FavorSize;
		else if( String == "FavorSpeed" ) rValue = Configuration::Optimization::FavorSpeed;
		else if( String == "Full"       ) rValue = Configuration::Optimization::Full;

	} // EnumFromString

} // Reflection
