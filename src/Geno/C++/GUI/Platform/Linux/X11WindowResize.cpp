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

#if defined( __linux__ )

#define _GLFW_X11
#define GLFW_EXPOSE_NATIVE_X11

#include "X11WindowResize.h"

#include <cassert>
#include <cstring>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <../src/internal.h>

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   // Size via keyboard
#define _NET_WM_MOVERESIZE_CANCEL           11   // Cancel operation

//////////////////////////////////////////////////////////////////////////

void ResizeWindow( GLFWwindow* pWindow, int Border )
{
	_GLFWwindow*        pHandle     = ( _GLFWwindow* )pWindow;
	Display*            pX11Display = glfwGetX11Display();
	XClientMessageEvent Event       = { };

	XUngrabPointer( pX11Display, 0 );
	XFlush( pX11Display );

	double CursorX, CursorY;
	int    WindowX, WindowY;
	glfwGetCursorPos( pWindow, &CursorX, &CursorY );
	glfwGetWindowPos( pWindow, &WindowX, &WindowY );

	Event.type         = ClientMessage;
	Event.window       = pHandle->x11.handle;
	Event.message_type = XInternAtom( pX11Display, "_NET_WM_MOVERESIZE", False );
	Event.format       = 32;
	Event.data.l[ 0 ]  = WindowX + CursorX;
	Event.data.l[ 1 ]  = WindowY + CursorY;
	Event.data.l[ 2 ]  = Border;
	Event.data.l[ 3 ]  = 0;
	Event.data.l[ 4 ]  = 0;

	XSendEvent( pX11Display, _glfw.x11.root, False, SubstructureRedirectMask | SubstructureNotifyMask, ( XEvent* )&Event );

} // ResizeWindow

#endif // __linux__ 
