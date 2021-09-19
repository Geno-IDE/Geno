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

#include <chrono>
#include <string>

#ifdef DEBUG
#define GENO_PROFILE2( Message, Count ) volatile Timer __t##Count( Message )
#define GENO_PROFILE1( Message, Count ) GENO_PROFILE2( Message, Count )
#define GENO_PROFILE( Message )         GENO_PROFILE1( Message, __COUNTER__ )
#else // DEBUG
#define GENO_PROFILE( Message )
#endif // !DEBUG

class Timer
{
public:
	//////////////////////////////////////////////////////////////////////////

	Timer( const std::string& Message );
	~Timer();

	//////////////////////////////////////////////////////////////////////////

	void Stop( void );

	//////////////////////////////////////////////////////////////////////////

private:
	std::chrono::high_resolution_clock::time_point m_Start;
	std::string                                    m_Message;
	bool                                           m_Stopped;

}; // Timer
