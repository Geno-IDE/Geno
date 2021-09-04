#if defined( __APPLE__ )
#include <GL/glew.h>

#include "MacosGenoContentView.h"

#include "GUI/Widgets/TitleBar.h"

@implementation GenoContentView

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow
{
	self = [super initWithGlfwWindow:initWindow];
	if( self != nil )
	{
		mainWindow   = mainGenoWindow;
		movingWindow = NO;
	}
	return self;
}

- ( void )setGLFWVariables
{
	id oldView = window->ns.view;
	window->ns.view = self;
	[window->ns.object setContentView:self];
	[window->ns.view setWantsBestResolutionOpenGLSurface:window->ns.retina];
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
		[self.window performWindowDragWithEvent:event];
		movingWindow = YES;
		return;
	}
	movingWindow = NO;
	[super mouseDown:event];
}

- ( void )mouseDragged:( NSEvent* )event
{
	if( movingWindow ) return;
	[super mouseDragged:event];
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

@end
#endif
