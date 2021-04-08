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

#include "PrimaryMonitor.h"

#include "ThirdParty/GLFW.h"

#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////

PrimaryMonitor::PrimaryMonitor( void )
{
	// Depends on GLFW to be initialized
	GLFW::Instance();

	glfwSetMonitorCallback( MonitorFunction );

	int Count;
	if( GLFWmonitor** ppMonitors = glfwGetMonitors( &Count ); ppMonitors != nullptr )
	{
		m_pMonitor = ppMonitors[ 0 ];

		for( int i = 0; i < Count; ++i )
			glfwSetMonitorUserPointer( ppMonitors[ i ], this );

		glfwGetMonitorWorkarea( m_pMonitor, &m_X, &m_Y, &m_Width, &m_Height );
	}

} // PrimaryMonitor

//////////////////////////////////////////////////////////////////////////

void PrimaryMonitor::MonitorFunction( GLFWmonitor* pMonitor, int /*Event*/ )
{
	if( PrimaryMonitor* pSelf = ( PrimaryMonitor* )glfwGetMonitorUserPointer( pMonitor ); pSelf != nullptr )
	{
		// Primary monitor may have changed
		int Count;
		if( GLFWmonitor** ppMonitors = glfwGetMonitors( &Count ); ppMonitors != nullptr )
		{
			pSelf->m_pMonitor = ppMonitors[ 0 ];

			glfwSetMonitorUserPointer( pSelf->m_pMonitor, pSelf );
			glfwGetMonitorWorkarea( pMonitor, &pSelf->m_X, &pSelf->m_Y, &pSelf->m_Width, &pSelf->m_Height );
		}
	}

} // MonitorFunction
