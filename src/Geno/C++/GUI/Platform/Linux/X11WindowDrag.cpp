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
#define _NET_WM_MOVERESIZE_MOVE 8 // Movement only

#include "X11WindowDrag.h"

#include <cassert>
#include <cstring>

#include <../src/internal.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//////////////////////////////////////////////////////////////////////////

void DragWindow( GLFWwindow* pWindow )
{
	_GLFWwindow*        pHandle     = ( _GLFWwindow* )pWindow;
	Display*            pX11Display = glfwGetX11Display();
	XClientMessageEvent Event       = { };

	XUngrabPointer( pX11Display, 0 );
	XFlush( pX11Display );

	Event.type         = ClientMessage;
	Event.window       = pHandle->x11.handle;
	Event.message_type = XInternAtom( pX11Display, "_NET_WM_MOVERESIZE", False );
	Event.format       = 32;
	Event.data.l[ 0 ]  = pHandle->x11.xpos + pHandle->x11.lastCursorPosX;
	Event.data.l[ 1 ]  = pHandle->x11.ypos + pHandle->x11.lastCursorPosY;
	Event.data.l[ 2 ]  = _NET_WM_MOVERESIZE_MOVE;
	Event.data.l[ 3 ]  = 0;
	Event.data.l[ 4 ]  = 0;

	XSendEvent( pX11Display, _glfw.x11.root, False, SubstructureRedirectMask | SubstructureNotifyMask, ( XEvent* )&Event );

} // DragWindow

#endif // __linux__
