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

#include "CompileJob.h"

#include <iostream>
#include <thread>

//////////////////////////////////////////////////////////////////////////

CompileJob::CompileJob( std::filesystem::path Path )
	: m_Path( std::move( Path ) )
{

} // CompileJob

//////////////////////////////////////////////////////////////////////////

void CompileJob::Run( void )
{
	std::cout << ( "Compiling: " + m_Path.string() + "\n" );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

} // Run
