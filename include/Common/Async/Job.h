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
#include "Common/Macros.h"

#include <functional>
#include <memory>
#include <vector>

class Job
{
	GENO_DISABLE_COPY_AND_MOVE( Job );

	friend class JobSystem;

//////////////////////////////////////////////////////////////////////////

public:

	template< typename Functor > explicit Job( Functor&& rrFunctor );

//////////////////////////////////////////////////////////////////////////

	void AddDependency( std::weak_ptr< Job > Job );
	bool CanRun       ( void ) const;

//////////////////////////////////////////////////////////////////////////

private:

	std::vector< std::weak_ptr< Job > > m_Dependencies       = { };
	std::function< void( void ) >       m_Function           = { };

	bool                                m_HasFinishedRunning = false;

}; // Job

//////////////////////////////////////////////////////////////////////////

template< typename Functor >
Job::Job( Functor&& rrFunctor )
	: m_Function( std::forward< Functor >( rrFunctor ) )
{

} // Job
