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

#include "TextBox.h"

#include "Editor/Widgets/Window.h"

GENO_NAMESPACE_BEGIN

TextBox::TextBox( const Window& parent )
	: hwnd_( NULL )
{
	HWND parent_handle = parent.GetNativeHandle();

	hwnd_ = CreateWindowExW( 0, L"EDIT", L"Hello, world!", WS_CHILDWINDOW | WS_TABSTOP | WS_VISIBLE | ES_LEFT | ES_MULTILINE, 10, 10, 128, 128, parent_handle, NULL, GetModuleHandleW( NULL ), this );
}

TextBox::TextBox( TextBox&& other )
	: hwnd_( other.hwnd_ )
{
	other.hwnd_ = NULL;
}

TextBox::~TextBox( void )
{
	if( hwnd_ )
		DestroyWindow( hwnd_ );
}

TextBox& TextBox::operator=( TextBox&& other )
{
	hwnd_       = other.hwnd_;

	other.hwnd_ = NULL;

	return *this;
}

GENO_NAMESPACE_END
