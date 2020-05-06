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
#include "Editor/Widgets/SubmenuItem.h"

#include <Windows.h>

#include <vector>

ALV_NAMESPACE_BEGIN

class Submenu
{
	ALV_DISABLE_COPY( Submenu );

public:

	using ItemVector = std::vector< SubmenuItem >;

public:

	 Submenu( void );
	 Submenu( Submenu&& other );
	~Submenu( void );

	Submenu& operator=( Submenu&& other );

public:

	void AddItem( SubmenuItem item );

public:

	HMENU GetNativeHandle( void ) const { return hmenu_; }

private:

	HMENU      hmenu_;

	ItemVector items_;

	UINT_PTR   next_item_id_;

};

ALV_NAMESPACE_END
