// TODO(MarcasRealAccount): Find fix for Maximize button not re rendering whilst resizing.
//                          Find fix for function not called when window edge is pressed initally until mouse moves.
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
	[super windowDidResize:notification];

	MainWindow::Instance().Render();
}

- ( void )windowWillStartLiveResize:( NSNotification* )notification
{
	if( resizeTimer ) return;
	double interval = 1.0 / window->videoMode.refreshRate;
	resizeTimer = [NSTimer timerWithTimeInterval:interval
                                         repeats:YES
                                           block:^ ( NSTimer* ) {
		MainWindow::Instance().Render();
	}];
	NSRunLoop* runLoop = [NSRunLoop currentRunLoop];
	[runLoop addTimer:resizeTimer
              forMode:NSRunLoopCommonModes];
}

- ( void )windowDidEndLiveResize:( NSNotification* )notification
{
	if( resizeTimer )
	{
		[resizeTimer invalidate];
		resizeTimer = nil;
	}
}

@end

@implementation GenoContentView

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow
{
	self = [super initWithGlfwWindow:initWindow];
	if( self != nil )
	{
		mainWindow        = mainGenoWindow;
		lastTitlebarClick = 0;
		lastTitlebarPos   = { 0.0f, 0.0f };
		movingWindow      = NO;
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

	[window->ns.object setStyleMask:[window->ns.object styleMask] | NSWindowStyleMaskResizable];
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
}

@end
#endif
