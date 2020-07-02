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

#include "Window.h"

#include "Geno/Gui/Platform/Win32/WindowClass.h"
#include "Geno/Gui/Widgets/MenuItem.h"
#include "Geno/Gui/Widgets/TextBox.h"

GENO_NAMESPACE_BEGIN

Window::Window( void )
{
	hwnd_ = CreateNativeHandle( NULL );
}

void Window::PollEvents( void )
{
	MSG msg;

	while( PeekMessageW( &msg, hwnd_, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessageW( &msg );
	}
}

void Window::SetMenu( Menu menu )
{
	::SetMenu( hwnd_, menu.GetNativeHandle() );

	menu_.emplace( std::move( menu ) );
}

bool Window::IsOpen( void ) const
{
	return IsWindow( hwnd_ );
}

LRESULT CALLBACK Window::WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	Window* self = ( Window* )GetWindowLongPtrW( hwnd, GWLP_USERDATA );

	self->HandleMessage( msg, wparam, lparam );

	return DefWindowProcW( hwnd, msg, wparam, lparam );
}

void Window::HandleMessage( UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
		case WM_SIZE:
		{
			WORD              width  = LOWORD( lparam );
			WORD              height = HIWORD( lparam );
			RECT              window_rect;
			WidgetRectChanged e;

			// Get window position before sending event
			if( GetWindowRect( hwnd_, &window_rect ) ) e.new_rect = Rect( Point( window_rect.left, window_rect.top ), Point( window_rect.right, window_rect.bottom ) );
			else                                       e.new_rect = Rect( width, height );

			Send( e );

			if( !children_.empty() )
			{
				double child_width = ( double )width / children_.size();

				for( size_t i = 0; i < children_.size(); ++i )
				{
					Rect rect;
					rect.min.x = ( int32_t )( child_width * i );
					rect.min.y = 0;
					rect.max.x = ( int32_t )( rect.min.x + child_width );
					rect.max.y = height;

					// Update children sizes
					children_[ i ].Get< Widget >().SetRect( rect );
				}
			}

		} break;

		case WM_COMMAND:
		{
			WORD notification_code = HIWORD( wparam );
			WORD identifier        = LOWORD( wparam );

			switch( notification_code )
			{
				case 0:
				{
					// Menu
				} break;

				case 1:
				{
					// Accelerator
				} break;

				default:
				{
					HWND control_window = ( HWND )lparam;

					HandleControlNotification( control_window, notification_code );

				} break;
			}

		} break;

		case WM_MENUCOMMAND:
		{
			if( !menu_ ) break;

			size_t      item_index  = ( size_t )wparam;
			HMENU       menu_handle = ( HMENU )lparam;
			const Menu* menu        = FindMenuByHandle( *menu_, menu_handle );

			if( menu )
				menu->GetItem( item_index ).OnClicked();

		} break;
	}
}

void Window::HandleControlNotification( HWND hwnd, WORD code )
{
	LONG_PTR user_data = GetWindowLongPtrW( hwnd, GWL_USERDATA );

	switch( code )
	{
		case EN_CHANGE:
		{
			TextBox* text_box = ( TextBox* )user_data;

			text_box->OnTextChanged();

		} break;
	}
}

const Menu* Window::FindMenuByHandle( Menu& which, HMENU hmenu ) const
{
	if( which.GetNativeHandle() == hmenu )
		return &which;

	for( size_t i = 0; i < which.GetItemCount(); ++i )
	{
		const MenuItem& item = which.GetItem( i );

		if( item.HasDropdownMenu() && item.GetDropdownMenu().GetNativeHandle() == hmenu )
			return &item.GetDropdownMenu();
	}

	return nullptr;
}

HWND Window::CreateNativeHandle( HWND parent ) const
{
	static Platform::WindowClass window_class( WndProc );

	DWORD style    = WS_OVERLAPPEDWINDOW;
	DWORD style_ex = WS_EX_OVERLAPPEDWINDOW;
	HWND  hwnd     = CreateWindowExW( style_ex, window_class.GetName(), NULL, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, GetModuleHandleW( NULL ), ( LPVOID )this );

	SetWindowLongPtrW( hwnd, GWLP_USERDATA, ( LONG_PTR )this );

	return hwnd;
}

GENO_NAMESPACE_END
