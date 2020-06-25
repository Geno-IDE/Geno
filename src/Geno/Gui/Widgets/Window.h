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
#include "Geno/Gui/Widgets/Menu.h"
#include "Geno/Gui/Widgets/Widget.h"

#include <optional>
#include <string_view>

#include <Windows.h>

GENO_NAMESPACE_BEGIN

class Window : public Widget
{
public:

	GENO_DISABLE_COPY( Window );
	GENO_DEFAULT_MOVE( Window );

public:

	Window( void );

public:

	void PollEvents( void );
	void SetMenu   ( Menu menu );

public:

	bool IsOpen( void ) const;

private:

	static LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

private:

	void HandleMessage             ( UINT msg, WPARAM wparam, LPARAM lparam );
	void HandleControlNotification ( HWND hwnd, WORD code );

private:

	const Menu* FindMenuByHandle( Menu& which, HMENU hmenu ) const;

private:

	std::optional< Menu > menu_;

};

GENO_NAMESPACE_END
