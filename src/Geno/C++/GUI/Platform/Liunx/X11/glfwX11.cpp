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

#if defined ( __linux__ )

#include <cassert>
#include <cstring>

#include <X11/Xlib.h>
#include "glfwX11.h"

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

GLFWAPI void glfwDragWindow( GLFWwindow* handle )
{
	XClientMessageEvent xclient;
	memset( &xclient, 0, sizeof( XClientMessageEvent ) );
	XUngrabPointer( _glfw.x11.display, 0 );
	XFlush( _glfw.x11.display );
	xclient.type = ClientMessage;
	xclient.window = window->x11.handle;
	xclient.message_type = XInternAtom( _glfw.x11.display, "_NET_WM_MOVERESIZE", False );
	xclient.format = 32;
	xclient.data.l[ 0 ] = window->x11.xpos + window->x11.lastCursorPosX;
	xclient.data.l[ 1 ] = window->x11.ypos + window->x11.lastCursorPosY;
	xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_MOVE;
	xclient.data.l[ 3 ] = 0;
	xclient.data.l[ 4 ] = 0;
	XSendEvent( _glfw.x11.display, _glfw.x11.root, False, SubstructureRedirectMask | SubstructureNotifyMask, ( XEvent* )&xclient );
}

GLFWAPI void glfwX11ResizeWindow( GLFWwindow* handle, int border )
{
	int winXpos, winYpos;
	double curXpos, curYpos;
	XClientMessageEvent xclient;
	memset( &xclient, 0, sizeof( XClientMessageEvent ) );
	XUngrabPointer( _glfw.x11.display, 0 );
	XFlush( _glfw.x11.display );
	glfwPlatformGetCursorPos( window, &curXpos, &curYpos );
	glfwPlatformGetWindowPos( window, &winXpos, &winYpos );
	xclient.type = ClientMessage;
	xclient.window = window->x11.handle;
	xclient.message_type = XInternAtom( _glfw.x11.display, "_NET_WM_MOVERESIZE", False );
	xclient.format = 32;
	xclient.data.l[ 0 ] = winXpos + curXpos;
	xclient.data.l[ 1 ] = winYpos + curYpos;
	switch( border )
	{
		case GLFW_WINDOW_LEFT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_LEFT;
			break;
		case GLFW_WINDOW_TOP:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOP;
			break;
		case GLFW_WINDOW_RIGHT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_RIGHT;
			break;
		case GLFW_WINDOW_BOTTOM:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOM;
			break;
		case GLFW_WINDOW_TOPLEFT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
			break;
		case GLFW_WINDOW_TOPRIGHT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
			break;
		case GLFW_WINDOW_BOTTOMLEFT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
			break;
		case GLFW_WINDOW_BOTTOMRIGHT:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
	}
	xclient.data.l[ 3 ] = 0;
	xclient.data.l[ 4 ] = 0;
	XSendEvent( _glfw.x11.display, _glfw.x11.root, False, SubstructureRedirectMask | SubstructureNotifyMask, ( XEvent* )&xclient );
}

#endif

