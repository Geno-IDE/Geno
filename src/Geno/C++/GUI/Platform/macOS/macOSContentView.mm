/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no pEvent will
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

#if defined( __APPLE__ )

#define _GLFW_COCOA

#include "macOSContentView.h"

#include "GUI/Widgets/TitleBar.h"

#include <GL/glew.h>
#include <../src/internal.h>

// TODO(MarcasRealAccount): Find fix for live resize not starting when pWindow edge is pressed initally until mouse moves.
//                          Find fix for pWindow not re rendering when fullscreen/zoom popup is visible.

//////////////////////////////////////////////////////////////////////////

@implementation WindowDelegate

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow
{
	if( ( self = [ super initWithGlfwWindow:pInitWindow ] ) )
	{
		pMainWindow  = pMainWindow;
		pResizeTimer = nil;
	}

	return self;

} // initWithMainWindow

//////////////////////////////////////////////////////////////////////////

- ( void )setGLFWVariables
{
	id OldDelegate      = pWindow->ns.delegate;
	pWindow->ns.delegate = self;

	[ pWindow->ns.object setDelegate:pWindow->ns.delegate ];
	[ OldDelegate release ];

} // setGLFWVariables

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidResize:( NSNotification* )pNotification
{
	// Set position of buttons
	NSButton* pCloseButton       = [ pWindow->ns.object standardWindowButton:NSWindowCloseButton ];
	NSButton* pMiniaturizeButton = [ pWindow->ns.object standardWindowButton:NSWindowMiniaturizeButton ];
	NSButton* pZoomButton        = [ pWindow->ns.object standardWindowButton:NSWindowZoomButton ];

	[ pCloseButton       setFrameOrigin:{  7.0f, [ pWindow->ns.view frame ].size.height - 26.0f } ];
	[ pMiniaturizeButton setFrameOrigin:{ 27.0f, [ pWindow->ns.view frame ].size.height - 26.0f } ];
	[ pZoomButton        setFrameOrigin:{ 47.0f, [ pWindow->ns.view frame ].size.height - 26.0f } ];
	[ super windowDidResize:pNotification ];

	MainWindow::Instance().Render();

} // windowDidResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowWillStartLiveResize:( NSNotification* )pNotification
{
	if( ResizeTimer )
		return;

	const double interval = 1.0 / pWindow->videoMode.refreshRate;
	pResizeTimer          = [ NSTimer timerWithTimeInterval:interval repeats:YES block:^( NSTimer* ) { MainWindow::Instance().Render(); } ];
	NSRunLoop* pRunLoop   = [ NSRunLoop currentRunLoop ];

	[ pRunLoop addTimer:ResizeTimer forMode:NSRunLoopCommonModes ];

} // windowWillStartLiveResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidEndLiveResize:( NSNotification* )pNotification
{
	if( ResizeTimer )
	{
		[ ResizeTimer invalidate ];
		pResizeTimer = nil;
	}

} // windowDidEndLiveResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidBecomeKey:( NSNotification* )pNotification
{
	[ super windowDidBecomeKey:pNotification ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowCloseButton ]       setEnabled:YES ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowMiniaturizeButton ] setEnabled:YES ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowZoomButton ]        setEnabled:YES ];

} // windowDidBecomeKey

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidResignKey:( NSNotification* )pNotification
{
	[ super windowDidResignKey:pNotification ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowCloseButton ]       setEnabled:NO ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowMiniaturizeButton ] setEnabled:NO ];
	[ [ pWindow->ns.object standardWindowButton:NSWindowZoomButton ]        setEnabled:NO ];

} // windowDidResignKey

@end // WindowDelegate

//////////////////////////////////////////////////////////////////////////

@implementation ContentView

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow
{
	if( ( self = [ super initWithGlfwWindow:pInitWindow ] ) )
	{
		pMainWindow                = pMainWindow;
		LastTitleBarClick          = 0;
		LastTitleBarPos            = { 0.0f, 0.0f };
		MovingWindow               = NO;
		MouseInsideStandardButtons = NO;
	}

	return self;

} // initWithMainWindow

//////////////////////////////////////////////////////////////////////////

- ( void )setGLFWVariables
{
	id OldView       = pWindow->ns.view;
	pWindow->ns.view = self;

	[ pWindow->ns.object           setContentView:self ];
	[ pWindow->ns.view             setWantsBestResolutionOpenGLSurface:pWindow->ns.retina ];
	[ pWindow->context.nsgl.object setView:self ];
	[ OldView release ];
	[ pWindow->ns.object setStyleMask:[ pWindow->ns.object styleMask ] | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable ];

	// Create standard pWindow buttons, the close, min and zoom (max) buttons
	pCloseButton       = [ NSWindow standardWindowButton:NSWindowCloseButton       forStyleMask:[ pWindow->ns.object styleMask ]];
	pMiniaturizeButton = [ NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:[ pWindow->ns.object styleMask ]];
	pZoomButton        = [ NSWindow standardWindowButton:NSWindowZoomButton        forStyleMask:[ pWindow->ns.object styleMask ]];

	// Add them to the content view
	[ self addSubview:pCloseButton ];
	[ self addSubview:pMiniaturizeButton ];
	[ self addSubview:pZoomButton ];

	// Set position of buttons
	[ pCloseButton       setFrameOrigin:{  7.0f, self.frame.size.height - 26.0f } ];
	[ pMiniaturizeButton setFrameOrigin:{ 27.0f, self.frame.size.height - 26.0f } ];
	[ pZoomButton        setFrameOrigin:{ 47.0f, self.frame.size.height - 26.0f } ];

} // setGLFWVariables

//////////////////////////////////////////////////////////////////////////

- ( void )dealloc
{
	[ super dealloc ];

} // dealloc

//////////////////////////////////////////////////////////////////////////

- ( void )mouseDown:( NSEvent* )pEvent
{
	const NSPoint Location = [ pEvent locationInWindow ];
	const NSRect  Rect     = [ self frame ];

	if( !ImGui::IsAnyItemHovered() && ( Location.y > ( Rect.size.height - pMainWindow->pTitleBar->Height() ) ) )
	{
		const NSPoint        ScreenPos = [ NSEvent mouseLocation ];
		const NSTimeInterval Timestamp = [ pEvent timestamp ];

		if( ( Timestamp - LastTitleBarClick ) < 1.0 && NSEqualPoints( ScreenPos, LastTitleBarPos ) )
		{
			LastTitleBarClick = 0;
			LastTitleBarPos   = { 0.0f, 0.0f };
			MovingWindow      = NO;

			MainWindow::Instance().Maximize();

			return;
		}

		LastTitleBarClick = Timestamp;
		LastTitleBarPos   = ScreenPos;
		MovingWindow      = YES;

		[ pWindow performWindowDragWithEvent:pEvent ];

		return;
	}

	MovingWindow = NO;

	[ super mouseDown:pEvent ];

} // mouseDown

//////////////////////////////////////////////////////////////////////////

- ( void )mouseUp:( NSEvent* )pEvent
{
	if( MovingWindow )
	{
		MovingWindow = NO;
		return;
	}

	[ super mouseUp:pEvent ];

} // mouseUp

//////////////////////////////////////////////////////////////////////////

- ( void )mouseMoved:( NSEvent* )pEvent
{
	if( MovingWindow )
		return;

	[ super mouseMoved:pEvent ];
	[ self updateButtons:pEvent ];

} // mouseMoved

//////////////////////////////////////////////////////////////////////////

- ( void )flagsChanged:( NSEvent* )pEvent
{
	[ super flagsChanged:pEvent ];
	[ self updateButtons:pEvent ];

} // flagsChanged

//////////////////////////////////////////////////////////////////////////

- ( BOOL )_mouseInGroup:( NSButton* )pButton
{
	return MouseInsideStandardButtons;

} // _mouseInGroup

//////////////////////////////////////////////////////////////////////////

- ( void )updateButtons:( NSEvent* )pEvent
{
	const NSPoint Location = [ pEvent locationInWindow ];
	const NSRect  Rect     = [ self frame ];

	MouseInsideStandardButtons      = Location.y > ( Rect.size.height - pMainWindow->pTitleBar->Height() ) && Location.x < 67.0f;
	pCloseButton.needsDisplay       = YES;
	pMiniaturizeButton.needsDisplay = YES;
	pZoomButton.needsDisplay        = YES;

} // updateButtons

@end // ContentView

#endif // __APPLE__
