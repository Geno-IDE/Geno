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

#include "GCL/Object.h"

#include <Common/Macros.h>

namespace GCL
{
	Object::Object( std::string_view key )
		: key_( key )
	{
	}

	Object::Object( Object&& other )
	{
		*this = std::move( other );
	}

	Object& Object::operator=( Object&& other )
	{
		key_ = other.key_;
		value_.swap( other.value_ );

		other.key_ = std::string_view();

		return *this;
	}

	void Object::SetString( std::string_view string )
	{
		StringType& underlying_string = value_.index() == 0 ? value_.emplace< StringType >() : std::get< StringType >( value_ );

		underlying_string = string;
	}

	void Object::AddArrayItem( std::string_view item )
	{
		ArrayType& underlying_array = value_.index() == 0 ? value_.emplace< ArrayType >() : std::get< ArrayType >( value_ );

		underlying_array.push_back( item );
	}

	void Object::AddChild( Object child )
	{
		TableType& underlying_table_vector = value_.index() == 0 ? value_.emplace< TableType >() : std::get< TableType >( value_ );

		underlying_table_vector.emplace_back( std::move( child ) );
	}

	Object::StringType Object::String( void ) const
	{
		return std::get< StringType >( value_ );
	}

	const Object::ArrayType& Object::Array( void ) const
	{
		return std::get< ArrayType >( value_ );
	}

	const Object::TableType& Object::Table( void ) const
	{
		return std::get< TableType >( value_ );
	}
}
