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

#pragma once
#include "Common/Macros.h"

#include <variant>
#include <vector>

namespace GCL
{
	class Object
	{
	public:

		GENO_DISABLE_COPY( Object );

	public:

		using StringType = std::string_view;
		using ArrayType  = std::vector< std::string_view >;
		using TableType  = std::vector< Object >;
		using Variant    = std::variant< std::monostate, StringType, ArrayType, TableType >;

	public:
	
		explicit Object( std::string_view key );
		         Object( Object&& other );

		template< typename T >
		Object( std::string_view key, std::in_place_type_t< T > )
			: key_  ( key )
			, value_( std::in_place_type< T > )
		{
		}

		Object& operator=( Object&& other );

	public:

		void SetString   ( std::string_view string );
		void AddArrayItem( std::string_view item );
		void AddChild    ( Object child );

	public:

		std::string_view Key     ( void ) const { return key_; }
		bool             IsNull  ( void ) const { return value_.index() == 0; }
		bool             IsString( void ) const { return value_.index() == 1; }
		bool             IsArray ( void ) const { return value_.index() == 2; }
		bool             IsTable ( void ) const { return value_.index() == 3; }

	public:

		StringType       String( void ) const;
		const ArrayType& Array ( void ) const;
		const TableType& Table ( void ) const;

	private:

		std::string_view key_;
		Variant          value_;
	
	};
}
