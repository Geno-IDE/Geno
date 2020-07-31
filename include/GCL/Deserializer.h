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
#include <filesystem>
#include <initializer_list>
#include <string_view>

namespace GCL
{
	class Deserializer
	{
	public:

		struct KeyedValues;
		struct KeyedValues
		{
			explicit KeyedValues( std::string_view value )
				: key_or_value( value )
			{
			}

			KeyedValues( std::string_view key, const KeyedValues* first, const KeyedValues* last )
				: key_or_value( key )
				, values      ( first, last )
			{
			}

			std::string_view                     key_or_value;
			std::initializer_list< KeyedValues > values;
		};

	public:

		using ValueCallback = void( * )( const KeyedValues& values, void* user );

	public:
	
		explicit Deserializer( const std::filesystem::path& path, ValueCallback value_callback, void* user );

	};
}
