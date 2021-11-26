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

#include "Compilers/ICompiler.h"

#include <iostream>
#include <thread>

//////////////////////////////////////////////////////////////////////////

CompileJob::CompileJob( Configuration Configuration, std::filesystem::path InputFile )
	: m_Configuration( std::move( Configuration ) )
	, m_InputFile    ( std::move( InputFile ) )
{

} // CompileJob

//////////////////////////////////////////////////////////////////////////

void CompileJob::Run( void )
{
	if( !m_Configuration.m_Compiler )
	{
		std::cerr << "Failed to compile " << m_InputFile << ". Compiler missing!\n";
		return;
	}

	m_Configuration.m_Compiler->Compile( m_Configuration, m_InputFile );

} // Run
