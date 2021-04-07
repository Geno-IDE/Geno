/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#if defined( _WIN32 )

#include "Common/Macros.h"

#include <Windows.h>

class Win32ProcessInfo
{
	GENO_DISABLE_COPY( Win32ProcessInfo );

//////////////////////////////////////////////////////////////////////////

public:

	 Win32ProcessInfo( void ) = default;
	 Win32ProcessInfo( Win32ProcessInfo&& other );
	~Win32ProcessInfo( void );

	Win32ProcessInfo& operator=( Win32ProcessInfo&& other );
	
//////////////////////////////////////////////////////////////////////////

	PROCESS_INFORMATION*       operator& ( void )       { return &m_UnderlyingData; }
	const PROCESS_INFORMATION* operator& ( void ) const { return &m_UnderlyingData; }
	PROCESS_INFORMATION*       operator->( void )       { return &m_UnderlyingData; }
	const PROCESS_INFORMATION* operator->( void ) const { return &m_UnderlyingData; }

//////////////////////////////////////////////////////////////////////////

private:

	PROCESS_INFORMATION m_UnderlyingData = { };
	
}; // Win32ProcessInfo

#endif // _WIN32
