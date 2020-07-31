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
#include <variant>

namespace GCL
{
	struct Table;

	using Value       = std::string_view;
	using Array       = std::vector< Value >;
	using TableVector = std::vector< Table >;

	struct Table
	{
		Table( void ) = default;
		Table( Table&& other );

		Table& operator=( Table&& other );

		std::string_view                                          key;
		std::variant< std::monostate, Value, Array, TableVector > value;
	};

	class Deserializer
	{
	public:

		using ValueCallback = void( * )( Value value, void* user );
		using TableCallback = void( * )( Table table, void* user );

	public:
	
		explicit Deserializer( const std::filesystem::path& path, ValueCallback value_callback, TableCallback table_callback, void* user );

	private:

		bool ParseLine( std::string_view line, int indent_level );

	private:

		static void AddValueToTableCallback( Value value, void* user );
		static void AddTableToTableCallback( Table table, void* user );

	private:

		std::string_view unparsed_;
		ValueCallback    value_callback_;
		TableCallback    table_callback_;
		void*            user_;

	};
}
