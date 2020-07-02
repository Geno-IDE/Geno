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
#include "Geno/Core/EventDispatcher.h"
#include "Geno/Gui/Widgets/Widget.h"

#include <string_view>
#include <string>

GENO_NAMESPACE_BEGIN

class Widget;

struct TextBoxTextChanged
{
	std::wstring new_text;
};

class TextBox : public Widget, public EventDispatcher< TextBox, TextBoxTextChanged >
{
public:

	GENO_DISABLE_COPY( TextBox );

	friend class Window;

public:

	using ThisEventDispatcher = EventDispatcher< TextBox, TextBoxTextChanged >;
	using Widget::operator<<=;
	using ThisEventDispatcher::operator<<=;

public:

	TextBox( void ) = default;
	TextBox( TextBox&& other );

	TextBox& operator=( TextBox&& other );

public:

	void SetText( std::wstring_view text );

public:

	std::wstring GetText( void ) const { return m_text; }

private:

	using ThisEventDispatcher::Send;

private:

	void OnTextChanged( void );

private:

	HWND CreateNativeHandle( HWND parent ) const override;

private:

	std::wstring m_text;

};

GENO_NAMESPACE_END
