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

#pragma once

#if defined ( __linux__ )

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#define _GLFW_X11
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
// How nice of glfw to hide this
#include <../src/internal.h>

GLFWAPI void glfwDragWindow( GLFWwindow* handle );
GLFWAPI void _glfwDragWindow( _GLFWwindow* window );


GLFWAPI void glfwX11ResizeWindow( GLFWwindow* handle, int border );
GLFWAPI void _glfwX11ResizeWindow( _GLFWwindow* window, int border );

#endif // __liunx__