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
#include <type_traits>

/* Function argument type deduction by courtesy of https://stackoverflow.com/a/35348334 */

template< typename Ret, typename Arg, typename... Rest >
constexpr Arg _FirstArgumentHelper( Ret( * )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg _FirstArgumentHelper( Ret( F::* )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg _FirstArgumentHelper( Ret( F::* )( Arg, Rest... ) const );

template< typename F >
constexpr decltype( _FirstArgumentHelper( &F::operator() ) ) _FirstArgumentHelper( F );

template< typename T >
using FirstArgumentType = decltype( _FirstArgumentHelper( std::declval< T >() ) );
