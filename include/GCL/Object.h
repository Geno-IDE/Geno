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

//////////////////////////////////////////////////////////////////////////

public:

//////////////////////////////////////////////////////////////////////////

	using StringType = std::string;
	using TableType  = std::vector< Object >;
	using Variant    = std::variant< std::monostate, StringType, TableType >;

//////////////////////////////////////////////////////////////////////////

	                       Object( void ) = default;
	explicit               Object( std::string Name );
	                       Object( Object&& other ) noexcept;
	template< typename T > Object( std::string Name, std::in_place_type_t< T > );

	Object& operator=( Object&& rrOther ) noexcept;

//////////////////////////////////////////////////////////////////////////

	void SetString( std::string String );
	void SetTable ( void );
	void AddChild ( Object Child );

//////////////////////////////////////////////////////////////////////////

	const StringType& String( void ) const;
	const TableType&  Table ( void ) const;
	bool              Empty ( void ) const;

//////////////////////////////////////////////////////////////////////////

	std::string_view Name    ( void ) const { return m_Name; }
	bool             IsNull  ( void ) const { return m_Value.index() == 0; }
	bool             IsString( void ) const { return m_Value.index() == 1; }
	bool             IsTable ( void ) const { return m_Value.index() == 2; }

//////////////////////////////////////////////////////////////////////////

	Object& operator[]( std::string_view Name );
	Object& operator= ( std::string String );
	bool    operator==( std::string_view String ) const;

//////////////////////////////////////////////////////////////////////////

private:

	std::string m_Name;
	Variant     m_Value;
	
}; // Object

}

//////////////////////////////////////////////////////////////////////////

template< typename T >
GCL::Object::Object( std::string Name, std::in_place_type_t< T > )
	: m_Name ( std::move( Name ) )
	, m_Value( std::in_place_type< T > )
{
} // Object
