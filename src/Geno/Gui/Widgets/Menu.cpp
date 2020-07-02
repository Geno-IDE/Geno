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

#include "Menu.h"

#include "Geno/Gui/Widgets/MenuItem.h"

GENO_NAMESPACE_BEGIN

Menu::Menu( void )
	: hmenu_( CreateMenu() )
	, items_{ }
{
	MENUINFO info;
	info.cbSize     = sizeof( MENUINFO );
	info.fMask      = MIM_MENUDATA | MIM_STYLE;
	info.dwStyle    = MNS_NOTIFYBYPOS;
	info.dwMenuData = ( ULONG_PTR )this;
	SetMenuInfo( hmenu_, &info );
}

Menu::Menu( Menu&& other )
	: hmenu_( other.hmenu_ )
	, items_( std::move( other.items_ ) )
{
	other.hmenu_ = NULL;

	MENUINFO info;
	info.cbSize     = sizeof( MENUINFO );
	info.fMask      = MIM_MENUDATA;
	info.dwMenuData = ( ULONG_PTR )this;
	SetMenuInfo( hmenu_, &info );
}

Menu::~Menu( void )
{
	if( hmenu_ )
		DestroyMenu( hmenu_ );
}

Menu& Menu::operator=( Menu&& other )
{
	hmenu_       = other.hmenu_;
	items_       = std::move( other.items_ );

	other.hmenu_ = NULL;

	return *this;
}

MenuItem& Menu::AddItem( MenuItem item )
{
	if( item.HasDropdownMenu() ) AppendMenuW( hmenu_, MF_STRING | MF_POPUP, ( UINT_PTR )item.GetDropdownMenu().GetNativeHandle(), item.GetName().data() );
	else                         AppendMenuW( hmenu_, MF_STRING,            ( UINT_PTR )items_.size(),                            item.GetName().data() );

	return items_.emplace_back( std::move( item ) );
}

void Menu::AddSeparator( void )
{
	AppendMenuW( hmenu_, MF_SEPARATOR, ( UINT_PTR )items_.size(), NULL );

	// Add empty item to make sure indices match
	items_.emplace_back( L"SEPARATOR" );
}

GENO_NAMESPACE_END
