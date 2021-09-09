// TODO(MarcasRealAccount): Find fix for live resize not starting when window edge is pressed initally until mouse moves.
//                          Find fix for window not re rendering when fullscreen/zoom popup is visible.
#if defined( __APPLE__ )
#include <GL/glew.h>

#include "MacosGenoContentView.h"

#include "GUI/Widgets/TitleBar.h"

@implementation GenoWindowDelegate

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow
{
	self = [super initWithGlfwWindow:initWindow];
	if( self != nil )
	{
		mainWindow  = mainGenoWindow;
		resizeTimer = nil;
	}
	return self;
}

- ( void )setGLFWVariables
{
	id oldDelegate = window->ns.delegate;
	window->ns.delegate = self;
	[window->ns.object setDelegate:window->ns.delegate];
	[oldDelegate release];
}

- ( void )windowDidResize:( NSNotification* )notification
{
	// Set position of buttons
	NSButton* closeButton       = [window->ns.object standardWindowButton:NSWindowCloseButton];
	NSButton* miniaturizeButton = [window->ns.object standardWindowButton:NSWindowMiniaturizeButton];
	NSButton* zoomButton        = [window->ns.object standardWindowButton:NSWindowZoomButton];
	[closeButton       setFrameOrigin:{ 7.0f,  [window->ns.view frame].size.height - 26.0f }];
	[miniaturizeButton setFrameOrigin:{ 27.0f, [window->ns.view frame].size.height - 26.0f }];
	[zoomButton        setFrameOrigin:{ 47.0f, [window->ns.view frame].size.height - 26.0f }];

	[super windowDidResize:notification];

	MainWindow::Instance().Render();
}

- ( void )windowWillStartLiveResize:( NSNotification* )notification
{
	if( resizeTimer ) return;
	double interval = 1.0 / window->videoMode.refreshRate;
	resizeTimer = [NSTimer timerWithTimeInterval:interval repeats:YES block:^ ( NSTimer* ) {
		MainWindow::Instance().Render();
	}];
	NSRunLoop* runLoop = [NSRunLoop currentRunLoop];
	[runLoop addTimer:resizeTimer forMode:NSRunLoopCommonModes];
}

- ( void )windowDidEndLiveResize:( NSNotification* )notification
{
	if( resizeTimer )
	{
		[resizeTimer invalidate];
		resizeTimer = nil;
	}
}

- ( void )windowDidBecomeKey:( NSNotification* )notification
{
	[super windowDidBecomeKey:notification];
	[[window->ns.object standardWindowButton:NSWindowCloseButton]       setEnabled:YES];
	[[window->ns.object standardWindowButton:NSWindowMiniaturizeButton] setEnabled:YES];
	[[window->ns.object standardWindowButton:NSWindowZoomButton]        setEnabled:YES];
}

- ( void )windowDidResignKey:( NSNotification* )notification
{
	[super windowDidResignKey:notification];
	[[window->ns.object standardWindowButton:NSWindowCloseButton]       setEnabled:NO];
	[[window->ns.object standardWindowButton:NSWindowMiniaturizeButton] setEnabled:NO];
	[[window->ns.object standardWindowButton:NSWindowZoomButton]        setEnabled:NO];
}

@end

@implementation GenoContentView

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow
{
	self = [super initWithGlfwWindow:initWindow];
	if( self != nil )
	{
		mainWindow                 = mainGenoWindow;
		lastTitlebarClick          = 0;
		lastTitlebarPos            = { 0.0f, 0.0f };
		movingWindow               = NO;
		mouseInsideStandardButtons = NO;
	}
	return self;
}

- ( void )setGLFWVariables
{
	id oldView = window->ns.view;
	window->ns.view = self;
	[window->ns.object           setContentView:self];
	[window->ns.view             setWantsBestResolutionOpenGLSurface:window->ns.retina];
	[window->context.nsgl.object setView:self];
	[oldView release];

	[window->ns.object setStyleMask:[window->ns.object styleMask] | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable];

	// Create standard window buttons, the close, min and zoom (max) buttons
	closeButton       = [NSWindow standardWindowButton:NSWindowCloseButton       forStyleMask:[window->ns.object styleMask]];
	miniaturizeButton = [NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:[window->ns.object styleMask]];
	zoomButton        = [NSWindow standardWindowButton:NSWindowZoomButton        forStyleMask:[window->ns.object styleMask]];

	// Add them to the content view
	[self addSubview:closeButton];
	[self addSubview:miniaturizeButton];
	[self addSubview:zoomButton];

	// Set position of buttons
	[closeButton       setFrameOrigin:{ 7.0f, self.frame.size.height - 26.0f }];
	[miniaturizeButton setFrameOrigin:{ 27.0f, self.frame.size.height - 26.0f }];
	[zoomButton        setFrameOrigin:{ 47.0f, self.frame.size.height - 26.0f }];
}

- ( void )dealloc
{
	[super dealloc];
}

- ( void )mouseDown:( NSEvent* )event
{
	const NSPoint pos  = [event locationInWindow];
	const NSRect  rect = [self frame];
	if( !ImGui::IsAnyItemHovered() && ( pos.y > ( rect.size.height - mainWindow->pTitleBar->Height() ) ) )
	{
		const NSPoint        screenPos = [NSEvent mouseLocation];
		const NSTimeInterval timestamp = [event timestamp];
		if( ( timestamp - lastTitlebarClick ) < 1.0 && NSEqualPoints( screenPos, lastTitlebarPos ) )
		{
			MainWindow::Instance().Maximize();
			lastTitlebarClick = 0;
			lastTitlebarPos   = { 0.0f, 0.0f };
			movingWindow      = NO;
			return;
		}
		[self.window performWindowDragWithEvent:event];
		lastTitlebarClick = timestamp;
		lastTitlebarPos   = screenPos;
		movingWindow      = YES;
		return;
	}
	movingWindow = NO;
	[super mouseDown:event];
}

- ( void )mouseUp:( NSEvent* )event
{
	if( movingWindow )
	{
		movingWindow = NO;
		return;
	}
	[super mouseUp:event];
}

- ( void )mouseMoved:( NSEvent* )event
{
	if( movingWindow ) return;
	[super mouseMoved:event];

	[self updateButtons:event];
}

- ( void )flagsChanged:( NSEvent* )event
{
	[super flagsChanged:event];

	[self updateButtons:event];
}

- ( BOOL )_mouseInGroup:( NSButton* )button
{
    return mouseInsideStandardButtons;
}

- ( void )updateButtons:( NSEvent* )event
{
	const NSPoint pos  = [event locationInWindow];
	const NSRect  rect = [self frame];
	mouseInsideStandardButtons = pos.y > ( rect.size.height - mainWindow->pTitleBar->Height() ) && pos.x < 67.0f;
	closeButton.needsDisplay = YES;
	miniaturizeButton.needsDisplay = YES;
	zoomButton.needsDisplay = YES;
}

@end
#endif
