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

#include "Configuration.h"

//////////////////////////////////////////////////////////////////////////

void Configuration::Override( const Configuration& rOther )
{
	if( rOther.m_Compiler     ) m_Compiler     = rOther.m_Compiler;
	if( rOther.m_Architecture ) m_Architecture = rOther.m_Architecture;
	if( rOther.m_Optimization ) m_Optimization = rOther.m_Optimization;

} // Override

//////////////////////////////////////////////////////////////////////////

Configuration::Architecture Configuration::HostArchitecture( void )
{

#if defined( _M_X64 ) || defined( __x86_64__ )
	return Architecture::x86_64;
#elif defined( _M_IX86 ) || defined( __i386__ ) // _M_X64 || __x86_64__
	return Architecture::x86;
#elif defined( __aarch64__ ) // _M_IX86 || __i386__
	return Architecture::Arm64;
#elif defined( _M_ARM ) || defined( __arm__ ) // __aarch64__
	return Architecture::Arm;
#endif // _M_ARM || __arm__

} // HostArchitecture
