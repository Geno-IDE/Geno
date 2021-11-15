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

// TODO(MarcasRealAccount): Find fix for live resize not starting when m_pWindow edge is pressed initally until mouse moves.
//                          Find fix for m_pWindow not re rendering when fullscreen/zoom popup is visible.

//////////////////////////////////////////////////////////////////////////

@implementation WindowDelegate

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow
{
	if( ( self = [ super initWithGlfwWindow:pInitWindow ] ) )
	{
		m_pMainWindow  = pMainWindow;
		m_pResizeTimer = nil;
	}

	return self;

} // initWithMainWindow

//////////////////////////////////////////////////////////////////////////

- ( void )setGLFWVariables
{
	id OldDelegate      = window->ns.delegate;
	window->ns.delegate = self;

	[ window->ns.object setDelegate:window->ns.delegate ];
	[ OldDelegate release ];

} // setGLFWVariables

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidResize:( NSNotification* )pNotification
{
	// Set position of buttons
	NSButton* pCloseButton       = [ window->ns.object standardWindowButton:NSWindowCloseButton ];
	NSButton* pMiniaturizeButton = [ window->ns.object standardWindowButton:NSWindowMiniaturizeButton ];
	NSButton* pZoomButton        = [ window->ns.object standardWindowButton:NSWindowZoomButton ];

	[ pCloseButton       setFrameOrigin:{  7.0f, [ window->ns.view frame ].size.height - 26.0f } ];
	[ pMiniaturizeButton setFrameOrigin:{ 27.0f, [ window->ns.view frame ].size.height - 26.0f } ];
	[ pZoomButton        setFrameOrigin:{ 47.0f, [ window->ns.view frame ].size.height - 26.0f } ];
	[ super windowDidResize:pNotification ];

	MainWindow::Instance().Render();

} // windowDidResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowWillStartLiveResize:( NSNotification* )pNotification
{
	if( m_pResizeTimer )
		return;

	const double interval = 1.0 / window->videoMode.refreshRate;
	m_pResizeTimer        = [ NSTimer timerWithTimeInterval:interval repeats:YES block:^( NSTimer* ) { MainWindow::Instance().Render(); } ];
	NSRunLoop* pRunLoop   = [ NSRunLoop currentRunLoop ];

	[ pRunLoop addTimer:m_pResizeTimer forMode:NSRunLoopCommonModes ];

} // windowWillStartLiveResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidEndLiveResize:( NSNotification* )pNotification
{
	if( m_pResizeTimer )
	{
		[ m_pResizeTimer invalidate ];
		m_pResizeTimer = nil;
	}

} // windowDidEndLiveResize

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidBecomeKey:( NSNotification* )pNotification
{
	[ super windowDidBecomeKey:pNotification ];
	[ [ window->ns.object standardWindowButton:NSWindowCloseButton ]       setEnabled:YES ];
	[ [ window->ns.object standardWindowButton:NSWindowMiniaturizeButton ] setEnabled:YES ];
	[ [ window->ns.object standardWindowButton:NSWindowZoomButton ]        setEnabled:YES ];

} // windowDidBecomeKey

//////////////////////////////////////////////////////////////////////////

- ( void )windowDidResignKey:( NSNotification* )pNotification
{
	[ super windowDidResignKey:pNotification ];
	[ [ window->ns.object standardWindowButton:NSWindowCloseButton ]       setEnabled:NO ];
	[ [ window->ns.object standardWindowButton:NSWindowMiniaturizeButton ] setEnabled:NO ];
	[ [ window->ns.object standardWindowButton:NSWindowZoomButton ]        setEnabled:NO ];

} // windowDidResignKey

@end // WindowDelegate

//////////////////////////////////////////////////////////////////////////

@implementation ContentView

- ( instancetype )initWithMainWindow:( MainWindow* )pMainWindow :( _GLFWwindow* )pInitWindow
{
	if( ( self = [ super initWithGlfwWindow:pInitWindow ] ) )
	{
		m_pMainWindow                = pMainWindow;
		m_LastTitleBarClick          = 0;
		m_LastTitleBarPos            = { 0.0f, 0.0f };
		m_MovingWindow               = NO;
		m_MouseInsideStandardButtons = NO;
	}

	return self;

} // initWithMainWindow

//////////////////////////////////////////////////////////////////////////

- ( void )setGLFWVariables
{
	id OldView      = window->ns.view;
	window->ns.view = self;

	[ window->ns.object           setContentView:self ];
	[ window->ns.view             setWantsBestResolutionOpenGLSurface:window->ns.retina ];
	[ window->context.nsgl.object setView:self ];
	[ OldView release ];
	[ window->ns.object setStyleMask:[ window->ns.object styleMask ] | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable ];

	// Create standard window buttons, the close, min and zoom (max) buttons
	m_pCloseButton       = [ NSWindow standardWindowButton:NSWindowCloseButton       forStyleMask:[ window->ns.object styleMask ]];
	m_pMiniaturizeButton = [ NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:[ window->ns.object styleMask ]];
	m_pZoomButton        = [ NSWindow standardWindowButton:NSWindowZoomButton        forStyleMask:[ window->ns.object styleMask ]];

	// Add them to the content view
	[ self addSubview:m_pCloseButton ];
	[ self addSubview:m_pMiniaturizeButton ];
	[ self addSubview:m_pZoomButton ];

	// Set position of buttons
	[ m_pCloseButton       setFrameOrigin:{  7.0f, self.frame.size.height - 26.0f } ];
	[ m_pMiniaturizeButton setFrameOrigin:{ 27.0f, self.frame.size.height - 26.0f } ];
	[ m_pZoomButton        setFrameOrigin:{ 47.0f, self.frame.size.height - 26.0f } ];

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

	if( !ImGui::IsAnyItemHovered() && ( Location.y > ( Rect.size.height - m_pMainWindow->pTitleBar->Height() ) ) )
	{
		const NSPoint        ScreenPos = [ NSEvent mouseLocation ];
		const NSTimeInterval Timestamp = [ pEvent timestamp ];

		if( ( Timestamp - m_LastTitleBarClick ) < 1.0 && NSEqualPoints( ScreenPos, m_LastTitleBarPos ) )
		{
			m_LastTitleBarClick = 0;
			m_LastTitleBarPos   = { 0.0f, 0.0f };
			m_MovingWindow      = NO;

			MainWindow::Instance().Maximize();

			return;
		}

		m_LastTitleBarClick = Timestamp;
		m_LastTitleBarPos   = ScreenPos;
		m_MovingWindow      = YES;

		[ self.window performWindowDragWithEvent:pEvent ];

		return;
	}

	m_MovingWindow = NO;

	[ super mouseDown:pEvent ];

} // mouseDown

//////////////////////////////////////////////////////////////////////////

- ( void )mouseUp:( NSEvent* )pEvent
{
	if( m_MovingWindow )
	{
		m_MovingWindow = NO;
		return;
	}

	[ super mouseUp:pEvent ];

} // mouseUp

//////////////////////////////////////////////////////////////////////////

- ( void )mouseMoved:( NSEvent* )pEvent
{
	if( m_MovingWindow )
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
	return m_MouseInsideStandardButtons;

} // _mouseInGroup

//////////////////////////////////////////////////////////////////////////

- ( void )updateButtons:( NSEvent* )pEvent
{
	const NSPoint Location = [ pEvent locationInWindow ];
	const NSRect  Rect     = [ self frame ];

	m_MouseInsideStandardButtons      = Location.y > ( Rect.size.height - m_pMainWindow->pTitleBar->Height() ) && Location.x < 67.0f;
	m_pCloseButton.needsDisplay       = YES;
	m_pMiniaturizeButton.needsDisplay = YES;
	m_pZoomButton.needsDisplay        = YES;

} // updateButtons

@end // ContentView

#endif // __APPLE__
