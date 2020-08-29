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
#include "Common/TypeTraits.h"

#include <functional>
#include <mutex>
#include <tuple>

template< typename Derived, typename... Types >
class EventDispatcher
{
public:

	GENO_DISABLE_COPY( EventDispatcher );

public:

	template< typename T >
	using Function = std::function< void( T ) >;

public:

	EventDispatcher( void )              = default;
	EventDispatcher( EventDispatcher&& ) = default;

	EventDispatcher& operator=( EventDispatcher&& ) = default;

public:

	/*
	 * Subscribe to event persistently (forever)
	 */
	template<
	  typename Functor,
	  typename = typename std::enable_if_t<
	    ( std::is_same_v< Types, std::remove_const_t< std::remove_reference_t< FirstArgumentType< Functor > > > > || ... )
	  >
	>
	Derived& operator<<=( Functor&& functor )
	{
		return Subscribe< Functor >( std::forward< Functor >( functor ), true );
	}

	/*
	 * Subscribe to event non-persistently (only once)
	 */
	template<
	  typename Functor,
	  typename = typename std::enable_if_t<
	    ( std::is_same_v< Types, std::remove_const_t< std::remove_reference_t< FirstArgumentType< Functor > > > > || ... )
	  >
	>
	Derived& operator^=( Functor&& functor )
	{
		return Subscribe< Functor >( std::forward< Functor >( functor ), false );
	}

protected:

	template< typename T >
	void Publish( const T& e )
	{
		SubscriberVector< T >   vec_copy;
		std::vector< uint64_t > non_persistent_subscriber_ids;

		{
			/** 
			 * We make a copy of the subscriber vector here in case any of the functors called adds another subscriber.
			 * This is a problem for two reasons.
			 * 1) It may resize the vector, in which case the iterator will be invalidated.
			 * 2) It will send *this* event to the new subscriber even though they subscribed after the event was published.
			 */
			MutexLock lock( mutex_ );
			vec_copy = std::get< SubscriberVector< T > >( subscribers_ );
		}

		for( auto it = vec_copy.begin(); it != vec_copy.end(); ++it )
		{
			it->function( e );

			if( !it->persistent )
				non_persistent_subscriber_ids.push_back( it->id );
		}

//////////////////////////////////////////////////////////////////////////

		MutexLock              lock( mutex_ );
		SubscriberVector< T >& vec_ref = std::get< SubscriberVector< T > >( subscribers_ );

		for( uint64_t id : non_persistent_subscriber_ids )
		{
			// Find the subscriber to be removed from the function pointer
			auto it = std::find_if( vec_ref.begin(), vec_ref.end(),
				[ id ]( const Subscriber< T >& s )
				{
					return ( s.id == id );
				}
			);

			// Remove the subscriber
			if( it != vec_ref.end() )
				vec_ref.erase( it );
		}
	}

private:

	template< typename T >
	struct Subscriber
	{
		Function< T > function;
		uint64_t      id;
		bool          persistent;
	};

	template< typename T >
	using SubscriberVector = std::vector< Subscriber< T > >;
	using SubscribersTuple = std::tuple< SubscriberVector< Types >... >;
	using Mutex            = std::recursive_mutex;
	using MutexLock        = std::scoped_lock< Mutex >;

private:

	template< typename Functor >
	Derived& Subscribe( Functor&& functor, bool persistent )
	{
		using T = std::remove_const_t< std::remove_reference_t< FirstArgumentType< Functor > > >;

		MutexLock              lock( mutex_ );
		SubscriberVector< T >& vec = std::get< SubscriberVector< T > >( subscribers_ );
		Subscriber< T >        new_subscriber;

		new_subscriber.function   = functor;
		new_subscriber.id         = ++next_id_;
		new_subscriber.persistent = persistent;

		vec.emplace_back( std::move( new_subscriber ) );

		return *static_cast< Derived* >( this );
	}

private:

	SubscribersTuple subscribers_;
	Mutex            mutex_;
	uint64_t         next_id_ = 0;

};
