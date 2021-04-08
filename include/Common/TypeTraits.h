/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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
#include <variant>

// Function argument type deduction by courtesy of https://stackoverflow.com/a/35348334

template< typename Ret, typename Arg, typename... Rest >
constexpr Arg FirstArgumentHelper( Ret( * )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg FirstArgumentHelper( Ret( F::* )( Arg, Rest... ) );

template< typename Ret, typename F, typename Arg, typename... Rest >
constexpr Arg FirstArgumentHelper( Ret( F::* )( Arg, Rest... ) const );

template< typename F >
constexpr decltype( FirstArgumentHelper( &F::operator() ) ) FirstArgumentHelper( F );

template< typename T >
using FirstArgumentType = decltype( FirstArgumentHelper( std::declval< T >() ) );

template< typename F, typename T >
constexpr bool IS_FIRST_ARGUMENT_SAME_AS = std::is_same_v< T, std::remove_cv_t< std::remove_reference_t< FirstArgumentType< F > > > >;

// Get type index within variant by courtesy of https://stackoverflow.com/a/52303687

template< typename >
struct Tag
{
}; // Tag

template< typename T, typename... Ts >
constexpr size_t VARIANT_INDEX = std::variant< Tag< Ts >... >( Tag< T >{ } ).index();

template< typename T, typename Variant >
struct UniqueIndex;

template< typename T, typename... Ts >
struct UniqueIndex< T, std::variant< Ts... > > : std::integral_constant< size_t, VARIANT_INDEX< T, Ts... > >
{
}; // UniqueIndex

template< typename T, typename... Ts >
constexpr auto UNIQUE_INDEX = UniqueIndex< T, Ts... >::value;
