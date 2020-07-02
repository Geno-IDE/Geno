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
#include "Geno/Geno.h"

#include <new>
#include <type_traits>
#include <utility>

GENO_NAMESPACE_BEGIN

class Any;

class Any
{
public:

	GENO_DISABLE_COPY( Any );

public:

	static constexpr size_t total_size = 128;

	using DtorFunc = void( Any::* )( void );
	using MoveFunc = void( Any::* )( Any&& );
	using CtorFunc = void( Any::* )( Any&& );
	using Storage  = std::aligned_storage_t< total_size - sizeof( DtorFunc ) - sizeof( MoveFunc ) - sizeof( CtorFunc ) >;

public:

	Any( void );
	Any( Any&& other );

	template< typename T, typename = typename std::enable_if_t< std::is_default_constructible_v< T > > >
	Any( std::in_place_type_t< T > )
		: storage_  { }
		, dtor_func_{ &Any::Dtor< T > }
		, move_func_{ &Any::Move< T > }
		, ctor_func_{ &Any::Ctor< T > }
	{
		static_assert( sizeof( T ) <= sizeof( Storage ), "type is too large" );

		( this->*ctor_func_ )( Any() );
	}

	template< typename T, typename = typename std::enable_if_t< std::is_move_constructible_v< T > > >
	Any( T&& value )
		: storage_  { }
		, dtor_func_{ &Any::Dtor< T > }
		, move_func_{ &Any::Move< T > }
		, ctor_func_{ &Any::Ctor< T > }
	{
		static_assert( sizeof( T ) <= sizeof( Storage ), "type is too large" );

		( this->*ctor_func_ )( std::move( value ) );
	}

	~Any( void );

	Any& operator=( Any&& other );

public:

	template< typename T >
	T& Get( void )
	{
		return *reinterpret_cast< T* >( &storage_ );
	}

	template< typename T >
	const T& Get( void ) const
	{
		return *reinterpret_cast< const T* >( &storage_ );
	}

private:

	template< typename T, typename = typename std::enable_if_t< std::is_move_constructible_v< T > > >
	void Ctor( Any&& other )
	{
		T* lhs = reinterpret_cast< T* >( &storage_ );
		T* rhs = reinterpret_cast< T* >( &other.storage_ );

		new( lhs ) T( std::move( *rhs ) );
	}

	template< typename T >
	void Dtor( void )
	{
		reinterpret_cast< T* >( &storage_ )->~T();
	}

	template< typename T, typename = typename std::enable_if_t< std::is_move_assignable_v< T > > >
	void Move( Any&& other )
	{
		T* lhs = reinterpret_cast< T* >( &storage_ );
		T* rhs = reinterpret_cast< T* >( &other.storage_ );

		*lhs = std::move( *rhs );
	}

private:

	Storage  storage_;
	CtorFunc ctor_func_;
	DtorFunc dtor_func_;
	MoveFunc move_func_;

};

GENO_NAMESPACE_END
