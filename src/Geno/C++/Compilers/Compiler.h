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
#include "Common/Aliases.h"
#include "Common/EventDispatcher.h"
#include "Common/Macros.h"

#include <atomic>
#include <filesystem>
#include <future>
#include <string_view>
#include <string>

struct CompilationDone
{
	std::filesystem::path path;

	int exit_code;
};

class Compiler : public EventDispatcher< Compiler, CompilationDone >
{
	GENO_SINGLETON( Compiler ) = default;

public:

	void Compile( const std::filesystem::path& path );
	void SetPath( path_view path );

private:

	struct Args
	{
		std::filesystem::path input;
		std::filesystem::path output;
	};

private:

	std::wstring MakeCommandLine( const Args& args );
	void         AsyncCB        ( Args args );

private:

	std::mutex                         path_mutex_;
	std::filesystem::path              path_;
	std::vector< std::future< void > > build_futures_;

};
