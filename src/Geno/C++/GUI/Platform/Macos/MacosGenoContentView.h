#pragma once

#include "GUI/MainWindow.h"

#define _GLFW_COCOA
#include <glfw/src/internal.h>
#undef _GLFW_COCOA

struct _GLFWwindow;

@interface GLFWContentView : NSView< NSTextInputClient >
{
	_GLFWwindow*               window;
	NSTrackingArea*            trackingArea;
	NSMutableAttributedString* markedText;
}

- ( instancetype )initWithGlfwWindow:( _GLFWwindow* )initWindow;

@end

@interface GenoContentView : GLFWContentView
{
	MainWindow* mainWindow;
	BOOL        movingWindow;
};

- ( instancetype )initWithMainWindow:( MainWindow* )mainGenoWindow :( _GLFWwindow* )initWindow;

- ( void )setGLFWVariables;

@end
