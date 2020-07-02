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

GENO_NAMESPACE_BEGIN

TextBox::TextBox( TextBox&& other )
	: Widget             ( std::move( other ) )
	, ThisEventDispatcher( std::move( other ) )
	, text_              ( std::move( other.text_ ) )
{
}

TextBox& TextBox::operator=( TextBox&& other ) = default;

void TextBox::SetText( std::wstring_view text )
{
	text_ = text;

	if( hwnd_ )
		SetWindowTextW( hwnd_, text_.data() );
}

void TextBox::OnTextChanged( void )
{
	text_.resize( GetWindowTextLengthW( hwnd_ ) );

	GetWindowTextW( hwnd_, &text_[ 0 ], std::numeric_limits< int >::max() );

	TextBoxTextChanged e;
	e.new_text = text_;

	Send( e );
}

HWND TextBox::CreateNativeHandle( HWND parent ) const
{
	DWORD style = WS_TABSTOP | WS_CHILD | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL;
	HWND  hwnd  = CreateWindowExW( 0, L"EDIT", L"", style, 0, 0, 64, 64, parent, NULL, NULL, ( LPVOID )this );

	SetWindowLongPtrW( hwnd, GWLP_USERDATA, ( LONG_PTR )this );

	// Apply text content that may have been set before the handle was created
	if( !text_.empty() )
		SetWindowTextW( hwnd, text_.data() );

	return hwnd;
}

GENO_NAMESPACE_END
