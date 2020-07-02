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

#include "Rect.h"

#include <type_traits>

GENO_NAMESPACE_BEGIN

Rect::Rect( const Rect& other )
	: min( other.min )
	, max( other.max )
{
}

Rect::Rect( Rect&& other )
	: min( std::move( other.min ) )
	, max( std::move( other.max ) )
{
}

Rect::Rect( uint32_t width, uint32_t height )
	: min( 0, 0 )
	, max( width, height )
{
}

Rect::Rect( Point min, Point max )
	: min( min )
	, max( max )
{
}

Rect& Rect::operator=( const Rect& other )
{
	min = other.min;
	max = other.max;

	return *this;
}

Rect& Rect::operator=( Rect&& other )
{
	min = std::move( other.min );
	max = std::move( other.max );

	return *this;
}

uint32_t Rect::Width( void ) const
{
	return ( uint32_t )( max.x - min.x );
}

uint32_t Rect::Height( void ) const
{
	return ( uint32_t )( max.y - min.y );
}

uint32_t Rect::Area( void ) const
{
	return ( Width() * Height() );
}

GENO_NAMESPACE_END
