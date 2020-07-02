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
#include "Geno/Core/STL/Any.h"
#include "Geno/Core/EventDispatcher.h"
#include "Geno/Core/Rect.h"

#include <Windows.h>

#include <cstdint>
#include <vector>

GENO_NAMESPACE_BEGIN

struct WidgetRectChanged
{
	Rect new_rect;
};

class Widget : public EventDispatcher< Widget, WidgetRectChanged >
{
public:

	GENO_DISABLE_COPY( Widget );

public:

	         Widget( void );
	         Widget( Widget&& other );
	virtual ~Widget( void );

	Widget& operator=( Widget&& other );

public:

	void Show   ( void );
	void Hide   ( void );
	void SetRect( const Rect& rect );

public:

	template<
	  typename T,
	  typename... Args,
	  typename = typename std::enable_if_t< std::is_base_of_v< Widget, T > >
	>
	T& AddChild( Args&&... args )
	{
		T child( std::forward< Args >( args )... );

		PrepareAddChild( child );

		return children_.emplace_back( std::move( child ) ).Get< T >();
	}

public:

	uint32_t Width  ( void ) const;
	uint32_t Height ( void ) const;
	bool     IsShown( void ) const;

public:

	HWND GetNativeHandle( void ) const { return hwnd_; }

protected:

	HWND               hwnd_;

	std::vector< Any > children_;

private:

	void PrepareAddChild( Widget& child );

};

GENO_NAMESPACE_END
