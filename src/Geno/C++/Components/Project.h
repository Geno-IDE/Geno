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
#include "Components/Configuration.h"

#include <GCL/Object.h>

#include <filesystem>
#include <vector>

class Project
{
public:

	Project( void ) = default;

public:

	void Build      ( void );
	void Deserialize( void );
	void SetLocation( const std::filesystem::path& location ) { location_ = location; }

public:

	std::filesystem::path RelativePath( const std::filesystem::path& path ) const;

public:

	std::string_view                      Name ( void ) { return name_; }
	std::vector< std::filesystem::path >& Files( void ) { return files_; }

private:

	static void GCLObjectCallback( GCL::Object object, void* user );

private:

	std::filesystem::path                location_;
	std::string                          name_;
	std::vector< std::filesystem::path > files_;
	std::vector< std::filesystem::path > includes_;
	std::vector< Configuration >         configrations_;

};
