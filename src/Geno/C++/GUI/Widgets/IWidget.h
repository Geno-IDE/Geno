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

#include "Auxiliary/jsonSerializer.h"
#include "WidgetCommands/CommandStack.h"

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

class IWidget
{
public:

	IWidget( const std::filesystem::path& rJsonFile );
	virtual ~IWidget( void ) = default;

//////////////////////////////////////////////////////////////////////////

	using KeyCombination = std::vector< int >;

	std::map< KeyCombination, std::string > m_KeyBindings = {};

//////////////////////////////////////////////////////////////////////////

protected:

	void Observe();

	void WriteKeyBindings( jsonSerializer& rSerializer );
	void ReadKeyBindings( const rapidjson::Value::ConstMemberIterator& rIt );

//////////////////////////////////////////////////////////////////////////

	virtual void WriteSettings( jsonSerializer& rSerializer )                     = 0;
	virtual void ReadSettings( const rapidjson::Value::ConstMemberIterator& rIt ) = 0;

//////////////////////////////////////////////////////////////////////////

	CommandStack m_UndoCommandStack = {};
	CommandStack m_RedoCommandStack = {};

	using Action = std::function< void( void ) >;

	std::map< std::string, Action > m_Actions = {};

	std::filesystem::path m_JsonFile = {};

}; // IWidget
