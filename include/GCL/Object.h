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
		GENO_DISABLE_COPY( Object );

	public:

		using StringType = std::string;
		using TableType  = std::vector< Object >;
		using Variant    = std::variant< std::monostate, StringType, TableType >;

	public:
	
		         Object( void ) = default;
		explicit Object( std::string_view name );
		         Object( Object&& other );

		template< typename T >
		Object( std::string_view name, std::in_place_type_t< T > )
			: name_ ( name )
			, value_( std::in_place_type< T > )
		{
		}

		Object& operator=( Object&& other );

	public:

		void SetString( std::string_view string );
		void SetTable ( void );
		void AddChild ( Object child );

	public:

		std::string_view Name    ( void ) const { return name_; }
		bool             IsNull  ( void ) const { return value_.index() == 0; }
		bool             IsString( void ) const { return value_.index() == 1; }
		bool             IsTable ( void ) const { return value_.index() == 2; }

	public:

		const StringType& String( void ) const;
		const TableType&  Table ( void ) const;
		bool              Empty ( void ) const;

	public:

		Object& operator[]( std::string_view name );
		Object& operator= ( std::string_view string );

	public:

		bool operator==( std::string_view string ) const;

	private:

		std::string name_;
		Variant     value_;
	
	};
}
