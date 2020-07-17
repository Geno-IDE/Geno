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

PrimaryMonitor::PrimaryMonitor( void )
	: monitor_( nullptr )
	, x_      ( 0 )
	, y_      ( 0 )
	, width_  ( 0 )
	, height_ ( 0 )
{
	// Depends on GLFW to be initialized
	GLFW::GetInstance();

	glfwSetMonitorCallback( MonitorFunction );

	int count;

	if( GLFWmonitor** monitors = glfwGetMonitors( &count ); monitors != nullptr )
	{
		monitor_ = monitors[ 0 ];

		for( int i = 0; i < count; ++i )
			glfwSetMonitorUserPointer( monitors[ i ], this );

		glfwGetMonitorWorkarea( monitor_, &x_, &y_, &width_, &height_ );
	}
}

PrimaryMonitor& PrimaryMonitor::GetInstance( void )
{
	static PrimaryMonitor primary_monitor;
	return primary_monitor;
}

void PrimaryMonitor::MonitorFunction( GLFWmonitor* monitor, int /*event*/ )
{
	if( PrimaryMonitor* self = ( PrimaryMonitor* )glfwGetMonitorUserPointer( monitor ); self != nullptr )
	{
		// Primary monitor may have changed
		int count = 0;

		if( GLFWmonitor** monitors = glfwGetMonitors( &count ); monitors != nullptr )
		{
			self->monitor_ = monitors[ 0 ];

			glfwSetMonitorUserPointer( self->monitor_, self );
			glfwGetMonitorWorkarea( monitor, &self->x_, &self->y_, &self->width_, &self->height_ );
		}
	}
}
