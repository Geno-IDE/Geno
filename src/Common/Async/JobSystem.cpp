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

#include "Common/Async/JobSystem.h"

#include "Common/Async/Job.h"

//////////////////////////////////////////////////////////////////////////

JobSystem::~JobSystem( void )
{
	StopThreads();

} // ~JobSystem

//////////////////////////////////////////////////////////////////////////

void JobSystem::StartThreads( size_t ThreadCount )
{
	StopThreads();
	m_Threads.clear();

	m_Running = true;

	for( size_t i = 0; i < ThreadCount; ++i )
		m_Threads.emplace_back( &JobSystem::ThreadEntry, this );

} // StartThreads

//////////////////////////////////////////////////////////////////////////

void JobSystem::StopThreads( void )
{
	m_Running = false;

	for( std::thread& rThread : m_Threads )
		rThread.join();

} // StopThreads

//////////////////////////////////////////////////////////////////////////

void JobSystem::ThreadEntry( void )
{
	while( m_Running )
	{
		while( !m_Jobs.empty() )
		{
			JobPtr Job;

			m_JobsMutex.lock();

			for( auto it = m_Jobs.begin(); it != m_Jobs.end(); ++it )
			{
				JobPtr& rJob = *it;

				if( rJob && rJob->CanRun() )
				{
					Job = rJob;
					m_Jobs.erase( it );
					break;
				}
			}

			m_JobsMutex.unlock();

			if( Job )
			{
				Job->m_Function();
				Job->m_HasFinishedRunning = true;
			}
		}

		// Always sleep a little bit to avoid exhausting the CPU
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

} // ThreadEntry
