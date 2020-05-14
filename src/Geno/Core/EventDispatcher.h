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
#include "Geno/Core/TypeTraits.h"

#include <functional>
#include <tuple>

GENO_NAMESPACE_BEGIN

template< typename Derived, typename... Types >
class EventDispatcher
{
public:

	GENO_DISABLE_COPY( EventDispatcher );
	GENO_DEFAULT_MOVE( EventDispatcher );

public:

	EventDispatcher( void ) = default;

public:

	template<
	  typename Functor,
	  typename = typename std::enable_if_t<
	    ( std::is_same_v< Types, std::remove_const_t< std::remove_reference_t< FirstArgumentType< Functor > > > > && ... )
	  >
	>
	Derived&& operator<<=( Functor&& functor )
	{
		using T = std::remove_const_t< std::remove_reference_t< FirstArgumentType< Functor > > >;

		SubscriberVector< T >& vec = std::get< SubscriberVector< T > >( subscribers_ );
		Subscriber< T >        new_subscriber;

		new_subscriber.function = functor;

		vec.emplace_back( std::move( new_subscriber ) );

		return std::forward< Derived >( *static_cast< Derived* >( this ) );
	}

protected:

	template< typename T >
	void Send( const T& e ) const
	{
		const SubscriberVector< T >& vec = std::get< SubscriberVector< T > >( subscribers_ );

		for( const Subscriber< T >& sub : vec )
		{
			sub.function( e );
		}
	}

private:

	template< typename T >
	struct Subscriber
	{
		std::function< void( T ) > function;
	};

	template< typename T >
	using SubscriberVector = std::vector< Subscriber< T > >;

private:

	std::tuple< SubscriberVector< Types >... > subscribers_;

};

GENO_NAMESPACE_END
