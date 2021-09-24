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

#include "X11WindowDrag.h"

// How nice of glfw to hide this
#include <../src/internal.h>

#include <cassert>
#include <cstring>

#define _NET_WM_MOVERESIZE_MOVE 8 /* movement only */

void DragWindow( GLFWwindow window )
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

#endif // __linux__ 
