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

#include "Auxiliary/StringAux.h"

//////////////////////////////////////////////////////////////////////////

std::vector< std::string > StringAux::SplitString( const std::string& rString, const std::string& rDelimiters )
{
	size_t Start = 0;
	size_t End = rString.find_first_of( rDelimiters );
	std::vector< std::string > Result;

	while ( End <= std::string::npos )
	{
		std::string token = rString.substr( Start, End - Start );
		
		if ( !token.empty() ) Result.push_back( token );

		if ( End == std::string::npos ) break;

		Start = End + 1;
		End = rString.find_first_of( rDelimiters, Start );
	}

	return Result;
	
} // SplitString
