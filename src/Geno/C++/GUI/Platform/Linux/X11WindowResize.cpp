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

#include "X11WindowResize.h"

#include <cassert>
#include <cstring>

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

void ResizeWindow( GLFWwindow* window, int border )
{
	_GLFWwindow* pHandle     = ( _GLFWwindow* )window;
	Display*     pX11Display = glfwGetX11Display();
	Window       X11Window   = glfwGetX11Window( window );

	int winXpos, winYpos;
	double curXpos, curYpos;

	XClientMessageEvent xclient;
	memset( &xclient, 0, sizeof( XClientMessageEvent ) );

	XUngrabPointer( pX11Display, 0 );
	XFlush( pX11Display );

	glfwGetCursorPos( window, &curXpos, &curYpos );
	glfwGetWindowPos( window, &winXpos, &winYpos );

	xclient.type         = ClientMessage;
	xclient.window       = pHandle->x11.handle;
	xclient.message_type = XInternAtom( pX11Display, "_NET_WM_MOVERESIZE", False );
	xclient.format       = 32;
	xclient.data.l[ 0 ]  = winXpos + curXpos;
	xclient.data.l[ 1 ]  = winYpos + curYpos;

	switch( border )
	{
		case 1:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_LEFT;
			break;
		case 2:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOP;
			break;
		case 3:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_RIGHT;
			break;
		case 4:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOM;
			break;
		case 5:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
			break;
		case 6:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
			break;
		case 7:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
			break;
		case 8:
			xclient.data.l[ 2 ] = _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
	}

	xclient.data.l[ 3 ] = 0;
	xclient.data.l[ 4 ] = 0;
	XSendEvent( pX11Display, X11Window, False, SubstructureRedirectMask | SubstructureNotifyMask, ( XEvent* )&xclient );
}

#endif // __linux__ 
