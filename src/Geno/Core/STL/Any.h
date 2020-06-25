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

class Any
{
public:

	GENO_DISABLE_COPY( Any );

	static constexpr size_t total_size = 64;

public:

	Any( void )
		: storage_         { }
		, destructor_func_ { nullptr }
		, move_func_       { nullptr }
	{
	}

	Any( Any&& other )
		: storage_         { }
		, destructor_func_ { other.destructor_func_ }
		, move_func_       { other.move_func_ }
	{
		move_func_( &storage_, &other.storage_ );
	}

	template< typename T, typename = typename std::enable_if< std::is_default_constructible_v< T > > >
	Any( std::in_place_type_t< T > )
		: storage_         { }
		, destructor_func_ { Destruct< T > }
		, move_func_       { Move< T > }
	{
		new( reinterpret_cast< T* >( &storage_ ) ) T();
	}

	template< typename T, typename = typename std::enable_if< std::is_move_constructible_v< T > > >
	Any( T value )
		: storage_         { }
		, destructor_func_ { Destruct< T > }
		, move_func_       { Move< T > }
	{
		new( reinterpret_cast< T* >( &storage_ ) ) T( std::move( value ) );
	}

	~Any( void )
	{
		if( destructor_func_ )
			destructor_func_( &storage_ );
	}

	Any& operator=( Any&& other )
	{
		destructor_func_ = other.destructor_func_;
		move_func_       = other.move_func_;

		move_func_( &storage_, &other.storage_ );

		return *this;
	}

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

	using DestructFunc = void( * )( void* );
	using MoveFunc     = void( * )( void*, void* );
	using Storage      = std::aligned_storage_t< total_size - sizeof( DestructFunc ) - sizeof( MoveFunc ) >;

private:

	template< typename T >
	static void Destruct( void* storage_ptr )
	{
		T* ptr = static_cast< T* >( storage_ptr );

		ptr->~T();
	}

	template< typename T, typename = typename std::enable_if< std::is_move_assignable_v< T > > >
	static void Move( void* lhs_storage_ptr, void* rhs_storage_ptr )
	{
		T* lhs = static_cast< T* >( lhs_storage_ptr );
		T* rhs = static_cast< T* >( rhs_storage_ptr );

		*lhs = std::move( *rhs );
	}

private:

	Storage      storage_;
	DestructFunc destructor_func_;
	MoveFunc     move_func_;

};

GENO_NAMESPACE_END
