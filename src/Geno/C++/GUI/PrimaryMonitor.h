/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
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
#include "Common/Macros.h"

struct GLFWmonitor;

class PrimaryMonitor
{
	GENO_SINGLETON( PrimaryMonitor );

public:

	int X     ( void ) const { return x_; }
	int Y     ( void ) const { return y_; }
	int Width ( void ) const { return width_; }
	int Height( void ) const { return height_; }

private:

	static void MonitorFunction( GLFWmonitor* monitor, int event );

private:

	GLFWmonitor* monitor_;

	int x_;
	int y_;
	int width_;
	int height_;

};
