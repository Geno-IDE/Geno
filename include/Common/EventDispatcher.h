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
	GENO_DISABLE_COPY( EventDispatcher );

//////////////////////////////////////////////////////////////////////////

public:

	template< typename T > using Function = std::function< void( T ) >;

//////////////////////////////////////////////////////////////////////////

	EventDispatcher( void )              = default;
	EventDispatcher( EventDispatcher&& ) = default;

	EventDispatcher& operator=( EventDispatcher&& ) = default;

//////////////////////////////////////////////////////////////////////////

	template< typename F > Derived& operator<<=( F&& Functor );
	template< typename F > Derived& operator^= ( F&& Functor );

//////////////////////////////////////////////////////////////////////////

protected:

	template< typename T > void Publish( const T& e );

//////////////////////////////////////////////////////////////////////////

private:

	template< typename T >
	struct Subscriber
	{
		Function< T > Function;
		uint64_t      ID;
		bool          Persistent;

	}; // Subscriber

//////////////////////////////////////////////////////////////////////////

	template< typename T >
	using SubscriberVector = std::vector< Subscriber< T > >;
	using SubscribersTuple = std::tuple< SubscriberVector< Types >... >;
	using Mutex            = std::recursive_mutex;
	using MutexLock        = std::scoped_lock< Mutex >;

//////////////////////////////////////////////////////////////////////////

	template< typename F > Derived& Subscribe( F&& Functor, bool Persistent );

//////////////////////////////////////////////////////////////////////////

	SubscribersTuple m_Subscribers;
	Mutex            m_Mutex;
	uint64_t         m_NextID = 1;

}; // EventDispatcher

//////////////////////////////////////////////////////////////////////////

template< typename Derived, typename... Types >
template< typename T >
void EventDispatcher< Derived, Types... >::Publish( const T& e )
{
	SubscriberVector< T >   VectorCopy;
	std::vector< uint64_t > NonPersistentSubscriberIDs;

	{
		/** 
			* We make a copy of the subscriber vector here in case any of the functors called adds another subscriber.
			* This is a problem for two reasons.
			* 1) It may resize the vector, in which case the iterator will be invalidated.
			* 2) It will send *this* event to the new subscriber even though they subscribed after the event was published.
			*/
		MutexLock Lock( m_Mutex );
		VectorCopy = std::get< SubscriberVector< T > >( m_Subscribers );
	}

	for( auto it = VectorCopy.begin(); it != VectorCopy.end(); ++it )
	{
		it->Function( e );

		if( !it->Persistent )
			NonPersistentSubscriberIDs.push_back( it->ID );
	}

	{
		MutexLock              Lock( m_Mutex );
		SubscriberVector< T >& rVector = std::get< SubscriberVector< T > >( m_Subscribers );

		for( uint64_t ID : NonPersistentSubscriberIDs )
		{
			// Find the subscriber to be removed from the function pointer
			auto It = std::find_if( rVector.begin(), rVector.end(),
				[ ID ]( const Subscriber< T >& rSubscriber )
				{
					return ( rSubscriber.ID == ID );
				}
			);

			// Remove the subscriber
			if( It != rVector.end() )
				rVector.erase( It );
		}
	}

} // Publish

//////////////////////////////////////////////////////////////////////////

template< typename Derived, typename... Types >
template< typename F >
Derived& EventDispatcher< Derived, Types... >::operator<<=( F&& Functor )
{
	static_assert( ( IS_FIRST_ARGUMENT_SAME_AS< F, Types > || ... ), "Parameter mismatch" );

	return Subscribe< F >( std::forward< F >( Functor ), true );

} // operator<<=

//////////////////////////////////////////////////////////////////////////

template< typename Derived, typename... Types >
template< typename F >
Derived& EventDispatcher< Derived, Types... >::operator^=( F&& Functor )
{
	static_assert( ( IS_FIRST_ARGUMENT_SAME_AS< F, Types > || ... ), "Parameter mismatch" );

	return Subscribe< F >( std::forward< F >( Functor ), false );

} // operator^=

//////////////////////////////////////////////////////////////////////////

template< typename Derived, typename... Types >
template< typename F >
Derived& EventDispatcher< Derived, Types... >::Subscribe( F&& Functor, bool Persistent )
{
	using T = std::remove_cv_t< std::remove_reference_t< FirstArgumentType< F > > >;

	MutexLock              Lock( m_Mutex );
	SubscriberVector< T >& Vector = std::get< SubscriberVector< T > >( m_Subscribers );
	Subscriber< T >        NewSubscriber;
	NewSubscriber.Function   = Functor;
	NewSubscriber.ID         = m_NextID++;
	NewSubscriber.Persistent = Persistent;

	Vector.emplace_back( std::move( NewSubscriber ) );

	return *static_cast< Derived* >( this );

} // Subscribe
