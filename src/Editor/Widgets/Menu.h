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
#include "Geno.h"

#include <string_view>
#include <vector>

#include <Windows.h>

GENO_NAMESPACE_BEGIN

class MenuItem;

class Menu
{
	GENO_DISABLE_COPY( Menu );

public:

	using ItemVector = std::vector< MenuItem >;

public:

	 Menu( void );
	 Menu( Menu&& other );
	~Menu( void );

	Menu& operator=( Menu&& other );

public:

	void AddItem     ( MenuItem item );
	void AddSeparator( void );

public:

	HMENU           GetNativeHandle( void )         const { return hmenu_; }
	size_t          GetItemCount   ( void )         const { return items_.size(); }
	MenuItem&       GetItem        ( size_t index )       { return items_[ index ]; }
	const MenuItem& GetItem        ( size_t index ) const { return items_[ index ]; }

private:

	HMENU      hmenu_;

	ItemVector items_;

	UINT_PTR   next_item_id_;

};

GENO_NAMESPACE_END

// Fixes circular dependency
#include "Editor/Widgets/MenuItem.h"
