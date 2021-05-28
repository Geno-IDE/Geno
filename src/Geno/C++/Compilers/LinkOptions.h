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
#include "Components/Project.h"

#include <filesystem>

struct LinkOptions
{
	enum Flags
	{
		LinkerFlagNoDefaultLibs = 0x01,
		LinkerFlagNoSymbolTable = 0x02,

	}; // Flags

//////////////////////////////////////////////////////////////////////////

	std::vector< std::filesystem::path > ObjectFiles;
	std::vector< std::string >           Libraries;
	std::filesystem::path                OutputFile;
	uint32_t                             Flags            = 0;
	Project::Kind                        Kind             = Project::Kind::Application;

}; // LinkOptions
