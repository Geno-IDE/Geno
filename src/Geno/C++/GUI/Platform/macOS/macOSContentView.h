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
#if defined( __APPLE__ )

#include "GUI/MainWindow.h"

#import <Cocoa/Cocoa.h>

struct _GLFWwindow;

//////////////////////////////////////////////////////////////////////////

@interface GLFWWindowDelegate : NSObject
{
	_GLFWwindow* window;
}

- ( instancetype )initWithGlfwWindow:( _GLFWwindow* )pInitWindow;

- ( void )windowDidResize:( NSNotification* )pNotification;
- ( void )windowDidBecomeKey:( NSNotification* )pNotification;
- ( void )windowDidResignKey:( NSNotification* )pNotification;

@end // GLFWWindowDelegate

//////////////////////////////////////////////////////////////////////////

@interface GLFWContentView : NSView< NSTextInputClient >
{
	_GLFWwindow*               window;
	NSTrackingArea*            trackingArea;
	NSMutableAttributedString* markedText;
}

- ( instancetype )initWithGlfwWindow:( _GLFWwindow* )pInitWindow;

@end // GLFWContentView

//////////////////////////////////////////////////////////////////////////

@interface WindowDelegate : GLFWWindowDelegate
{
	MainWindow* m_pMainWindow;
	NSTimer*    m_pResizeTimer;
}

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow;
- ( void )setGLFWVariables;

@end // WindowDelegate

//////////////////////////////////////////////////////////////////////////

@interface ContentView : GLFWContentView
{
	MainWindow*    m_pMainWindow;
	NSTimeInterval m_LastTitleBarClick;
	NSPoint        m_LastTitleBarPos;
	BOOL           m_MovingWindow;
	NSButton*      m_pCloseButton;
	NSButton*      m_pMiniaturizeButton;
	NSButton*      m_pZoomButton;
	BOOL           m_MouseInsideStandardButtons;
};

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow;
- ( void )setGLFWVariables;

@end // ContentView

#endif // __APPLE__
