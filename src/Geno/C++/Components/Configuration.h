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
#include "Compilers/ICompiler.h"

#include <Common/Macros.h>

#include <optional>
#include <memory>

class Configuration
{
	GENO_DISABLE_COPY( Configuration );
	GENO_DEFAULT_MOVE( Configuration );

//////////////////////////////////////////////////////////////////////////

public:

	enum class Optimization
	{
		FavorSize,
		FavorSpeed,
		Full,

	}; // Optimization

//////////////////////////////////////////////////////////////////////////

	Configuration( void ) = default;

//////////////////////////////////////////////////////////////////////////

	void Override( const Configuration& rOther );

//////////////////////////////////////////////////////////////////////////

	std::shared_ptr< ICompiler >  m_Compiler;
	std::optional< Optimization > m_Optimization;

}; // Configuration

//////////////////////////////////////////////////////////////////////////

namespace Reflection
{

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

constexpr void EnumFromString( std::string_view String, Configuration::Optimization& rValue )
{
	if(      String == "FavorSize"  ) rValue = Configuration::Optimization::FavorSize;
	else if( String == "FavorSpeed" ) rValue = Configuration::Optimization::FavorSpeed;
	else if( String == "Full"       ) rValue = Configuration::Optimization::Full;

} // EnumFromString

} // Reflection
