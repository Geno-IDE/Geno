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
#include "Common/Async/Job.h"
#include "Common/Macros.h"

#include <deque>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

class JobSystem
{
	GENO_SINGLETON( JobSystem );

//////////////////////////////////////////////////////////////////////////

public:

	using JobPtr = std::shared_ptr< Job >;

//////////////////////////////////////////////////////////////////////////

	 JobSystem( void ) = default;
	~JobSystem( void );

//////////////////////////////////////////////////////////////////////////

	void StartThreads( size_t ThreadCount );

//////////////////////////////////////////////////////////////////////////

	template< typename Functor > JobPtr NewJob( Functor&& rrFunctor, std::span< JobPtr > Dependencies = { } );

//////////////////////////////////////////////////////////////////////////

private:

	void StopThreads( void );
	void ThreadEntry( void );

//////////////////////////////////////////////////////////////////////////

	std::vector< std::thread > m_Threads   = { };
	std::deque< JobPtr >       m_Jobs      = { };
	std::mutex                 m_JobsMutex = { };

	bool                       m_Running   = false;

}; // JobSystem

//////////////////////////////////////////////////////////////////////////

template< typename Functor >
JobSystem::JobPtr JobSystem::NewJob( Functor&& rrFunctor, std::span< JobPtr > Dependencies )
{
	std::scoped_lock Lock( m_JobsMutex );
	std::shared_ptr  Job = std::make_shared< ::Job >( std::forward< Functor >( rrFunctor ) );

	for( JobPtr& rDependency : Dependencies )
		Job->AddDependency( rDependency );

	m_Jobs.push_back( Job );

	return Job;

} // NewJob
