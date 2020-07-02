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

#include <cassert>

GENO_NAMESPACE_BEGIN

Widget::Widget( void )
	: hwnd_    ( NULL )
	, children_{ }
	, rect_    { }
	, shown_   ( true )
{
}

Widget::Widget( Widget&& other )
	: EventDispatcher( std::move( other ) )
	, hwnd_          ( other.hwnd_ )
	, children_      ( std::move( other.children_ ) )
	, rect_          ( std::move( other.rect_ ) )
	, shown_         ( other.shown_ )
{
	other.hwnd_  = NULL;
	other.shown_ = false;

	if( hwnd_ )
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

	if( hwnd_ )
		SetWindowLongPtrW( hwnd_, GWLP_USERDATA, ( LONG_PTR )this );

	return *this;
}

void Widget::Show( void )
{
	shown_ = true;

	if( hwnd_ )
		ShowWindow( hwnd_, SW_SHOW );

	for( Any& child : children_ )
		child.Get< Widget >().Show();
}

void Widget::Hide( void )
{
	shown_ = false;

	if( hwnd_ )
		ShowWindow( hwnd_, SW_HIDE );

	for( Any& child : children_ )
		child.Get< Widget >().Hide();
}

void Widget::SetRect( const Rect& rect )
{
	rect_ = rect;

	if( hwnd_ )
	{
		if( !MoveWindow( hwnd_, rect.min.x, rect.min.y, rect.Width(), rect.Height(), TRUE ) )
			return;
	}

	WidgetRectChanged e;
	e.new_rect = rect_;

	Send( e );
}

void Widget::PrepareAddChild( Widget& child )
{
	if( child.hwnd_ )
	{
		DWORD style = GetWindowLongW( child.hwnd_, GWL_STYLE );
		style &= ~WS_POPUP;
		style &= ~WS_CAPTION;
		style |=  WS_CHILD;

		SetWindowLongW( child.hwnd_, GWL_STYLE, style );
		SetParent( child.hwnd_, hwnd_ );
	}
	else
	{
		child.hwnd_ = child.CreateNativeHandle( hwnd_ );

		// Assume that the rect hasn't been initialized if area is 0
		if( child.rect_.Area() == 0 )
		{
			RECT window_rect;
			if( GetWindowRect( child.hwnd_, &window_rect ) )
				child.rect_ = Rect( Point( window_rect.left, window_rect.top ), Point( window_rect.right, window_rect.bottom ) );
		}

		// Apply states that may have been set before handle was created
		MoveWindow( child.hwnd_, child.rect_.min.x, child.rect_.min.y, child.rect_.Width(), child.rect_.Height(), TRUE );
		ShowWindow( child.hwnd_, child.shown_ ? SW_SHOW : SW_HIDE );
	}
}

GENO_NAMESPACE_END
