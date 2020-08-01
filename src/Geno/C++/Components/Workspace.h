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
#include "Components/BuildMatrix.h"
#include "Components/Project.h"

#include <GCL/Deserializer.h>

#include <filesystem>
#include <string>
#include <vector>

class Workspace
{
public:

	Workspace( void ) = default;

public:

	void Build      ( void );
	void Serialize  ( void );
	void Deserialize( void );
	void SetLocation( const std::filesystem::path& location ) { location_ = location; }

public:

	std::filesystem::path& Location( void )       { return location_; }
	std::string_view       Name    ( void ) const { return name_; }
	BuildMatrix&           Matrix  ( void )       { return build_matrix_; }

private:

	static void GCLObjectCallback( GCL::Object object, void* user );

private:

	std::filesystem::path  location_;
	std::vector< Project > projects_;
	std::string            name_;

	BuildMatrix            build_matrix_;

};
