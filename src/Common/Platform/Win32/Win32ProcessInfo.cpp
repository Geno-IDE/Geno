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

#if defined( _WIN32 )

#include "Common/Platform/Win32/Win32ProcessInfo.h"

//////////////////////////////////////////////////////////////////////////

Win32ProcessInfo::Win32ProcessInfo( Win32ProcessInfo&& rrOther ) noexcept
	: m_UnderlyingData( rrOther.m_UnderlyingData )
{
	ZeroMemory( &rrOther.m_UnderlyingData, sizeof( PROCESS_INFORMATION ) );

} // Win32ProcessInfo

//////////////////////////////////////////////////////////////////////////

Win32ProcessInfo::~Win32ProcessInfo( void )
{
	if( m_UnderlyingData.hThread )
		CloseHandle( m_UnderlyingData.hThread );

	if( m_UnderlyingData.hProcess )
		CloseHandle( m_UnderlyingData.hProcess );

} // ~Win32ProcessInfo

//////////////////////////////////////////////////////////////////////////

Win32ProcessInfo& Win32ProcessInfo::operator=( Win32ProcessInfo&& rrOther ) noexcept
{
	// Close existing handles
	this->~Win32ProcessInfo();

	m_UnderlyingData = rrOther.m_UnderlyingData;
	
	ZeroMemory( &rrOther.m_UnderlyingData, sizeof( PROCESS_INFORMATION ) );
	
	return *this;

} // operator=

#endif // _WIN32
