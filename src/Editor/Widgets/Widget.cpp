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

#include "Widget.h"

GENO_NAMESPACE_BEGIN

Widget::Widget( void )
	: hwnd_( NULL )
{
}

Widget::Widget( Widget&& other )
	: hwnd_( other.hwnd_ )
{
	other.hwnd_ = NULL;

	SetWindowLongPtrW( hwnd_, GWL_USERDATA, ( LONG_PTR )this );
}

Widget::~Widget( void )
{
	if( hwnd_ )
		DestroyWindow( hwnd_ );
}

Widget& Widget::operator=( Widget&& other )
{
	hwnd_       = other.hwnd_;

	other.hwnd_ = NULL;

	return *this;
}

void Widget::Show( void )
{
	ShowWindow( hwnd_, SW_SHOW );

	for( Widget& child : children_ )
		child.Show();
}

void Widget::Hide( void )
{
	ShowWindow( hwnd_, SW_HIDE );

	for( Widget& child : children_ )
		child.Hide();
}

uint32_t Widget::Width( void ) const
{
	RECT rect;

	if( GetWindowRect( hwnd_, &rect ) )
		return rect.right - rect.left;

	return 0;
}

uint32_t Widget::Height( void ) const
{
	RECT rect;

	if( GetWindowRect( hwnd_, &rect ) )
		return rect.bottom - rect.top;

	return 0;
}

GENO_NAMESPACE_END
