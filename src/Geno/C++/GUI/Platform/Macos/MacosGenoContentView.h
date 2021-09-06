#pragma once

#include "GUI/MainWindow.h"

#define _GLFW_COCOA
#include <glfw/src/internal.h>
#undef _GLFW_COCOA

struct _GLFWwindow;

@interface GLFWWindowDelegate : NSObject
{
	_GLFWwindow* window;
}

- ( instancetype )initWithGlfwWindow:( _GLFWwindow* )initWindow;

- ( void )windowDidResize:( NSNotification* )notification;

@end

@interface GLFWContentView : NSView< NSTextInputClient >
{
	_GLFWwindow*               window;
	NSTrackingArea*            trackingArea;
	NSMutableAttributedString* markedText;
}

- ( instancetype )initWithGlfwWindow:( _GLFWwindow* )initWindow;

@end

@interface GenoWindowDelegate : GLFWWindowDelegate
{
	MainWindow* mainWindow;
	NSTimer*    resizeTimer;
}

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow;

- ( void )setGLFWVariables;

@end

@interface GenoContentView : GLFWContentView
{
	MainWindow*    mainWindow;
	NSTimeInterval lastTitlebarClick;
	NSPoint        lastTitlebarPos;
	BOOL           movingWindow;
};

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow;

- ( void )setGLFWVariables;

@end
