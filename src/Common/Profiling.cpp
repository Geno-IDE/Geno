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

#include "Common/Profiling.h"

#include <iostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////

Timer::Timer( const std::string& Message )
	: m_Start( std::chrono::high_resolution_clock::now() )
	, m_Message( Message )
	, m_Stopped( false )
{
} // Timer

//////////////////////////////////////////////////////////////////////////

Timer::~Timer()
{
	if( !m_Stopped )
		Stop();

} // ~Timer

//////////////////////////////////////////////////////////////////////////

void Timer::Stop( void )
{
	float Diff = ( float )std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::high_resolution_clock::now() - m_Start ).count();

	std::stringstream Buffer;

	Buffer << m_Message << " ";

	if( Diff >= 1000000.0f )
	{
		Buffer << Diff / 1000000 << "s\n";
	}
	else if( Diff >= 1000.0f )
	{
		Buffer << Diff / 1000.0f << "ms\n";
	}
	else
	{
		Buffer << Diff << "us\n";
	}

	std::cout << Buffer.str();

	m_Stopped = false;

} // Stop
