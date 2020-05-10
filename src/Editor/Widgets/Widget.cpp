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
	: hwnd_    ( NULL )
	, children_{ }
{
}

Widget::Widget( Widget&& other )
	: hwnd_    ( other.hwnd_ )
	, children_( std::move( other.children_ ) )
{
	other.hwnd_ = NULL;

	SetWindowLongPtrW( hwnd_, GWLP_USERDATA, ( LONG_PTR )this );
}

Widget::~Widget( void )
{
	// Destroy children before invalidating the handle
	children_.clear();

	if( hwnd_ )
		DestroyWindow( hwnd_ );
}

Widget& Widget::operator=( Widget&& other )
{
	hwnd_       = other.hwnd_;
	children_   = std::move( other.children_ );

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

void Widget::AddChild( Widget child )
{
	DWORD style = GetWindowLongW( child.hwnd_, GWL_STYLE );
	style &= ~WS_POPUP;
	style &= ~WS_CAPTION;
	style |=  WS_CHILD;

	SetWindowLongW( child.hwnd_, GWL_STYLE, style );
	SetParent( child.hwnd_, hwnd_ );

	// Set visibility depending on new parent
	if( IsShown() ) child.Show();
	else            child.Hide();

	children_.emplace_back( std::move( child ) );
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

bool Widget::IsShown( void ) const
{
	return ( GetWindowLongW( hwnd_, GWL_STYLE ) & WS_VISIBLE ) != 0;
}

GENO_NAMESPACE_END
