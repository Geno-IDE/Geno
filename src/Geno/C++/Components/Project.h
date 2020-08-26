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
#include "Components/Enums/ProjectKind.h"
#include "Components/Configuration.h"

#include <Common/EventDispatcher.h>
#include <GCL/Object.h>

#include <filesystem>
#include <vector>

class ICompiler;
class Project;

struct ProjectBuildFinished
{
	Project* project;
	bool     success;
};

class Project : public EventDispatcher< Project, ProjectBuildFinished >
{
	GENO_DISABLE_COPY( Project );

public:

	static constexpr std::string_view ext = ".gprj";

public:

	explicit Project( std::filesystem::path location );
	Project( Project&& other );

	Project& operator=( Project&& other );

public:

	void Build      ( ICompiler& compiler );
	bool Serialize  ( void );
	bool Deserialize( void );

public:

	ProjectKind                          kind_ = ProjectKind::Unknown;

	std::filesystem::path                location_;
	std::string                          name_;
	std::vector< std::filesystem::path > files_;
	std::vector< std::filesystem::path > includes_;
	std::vector< Configuration >         configrations_;
	std::vector< std::filesystem::path > files_left_to_build_;

private:

	static void GCLObjectCallback( GCL::Object object, void* user );

private:

	void BuildNextFile( ICompiler& compiler );

};
