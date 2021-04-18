/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

template< typename Sender, typename TheoreticalFunction >
class Event
{
}; // Event

template< typename Sender, typename... Parameters >
class Event< Sender, void( Parameters... ) >
{
public:

	using ReceiverFunction = std::function< void( Sender&, Parameters... ) >;
	using ReceiverVector   = std::vector< ReceiverFunction >;

//////////////////////////////////////////////////////////////////////////

	template< typename Functor > void operator+=( Functor&& rrFunctor );
	                             void operator()( Sender& rSender, Parameters... Params );

//////////////////////////////////////////////////////////////////////////

private:

	ReceiverVector m_Receivers;

}; // Event

//////////////////////////////////////////////////////////////////////////

template< typename Sender, typename... Parameters >
template< typename Functor >
void Event< Sender, void( Parameters... ) >::operator+=( Functor&& rrFunctor )
{
	m_Receivers.emplace_back( std::forward< Functor >( rrFunctor ) );

} // operator+=

//////////////////////////////////////////////////////////////////////////

template< typename Sender, typename... Parameters >
void Event< Sender, void( Parameters... ) >::operator()( Sender& rSender, Parameters... Params )
{
	ReceiverVector ReceiversCopy = m_Receivers;

	for( ReceiverFunction& rReceiver : ReceiversCopy )
	{
		std::invoke( rReceiver, rSender, Params... );
	}

	while( !ReceiversCopy.empty() )
	{
		ReceiverVector::iterator IteratorToErase = std::find_if( m_Receivers.begin(), m_Receivers.end(),
			[ &ReceiversCopy ]( const ReceiverFunction& rReceiver )
			{
				return ( rReceiver.target_type() == ReceiversCopy.back().target_type() );
			}
		);

		m_Receivers.erase( IteratorToErase );
		ReceiversCopy.pop_back();
	}

} // operator()
