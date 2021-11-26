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
#include "Components/Configuration.h"
#include "Components/Project.h"
#include "Jobs/CompileJob.h"

#include <Common/Async/IJob.h>

#include <span>
#include <string>

class LinkJob final : public IJob
{
public:

	LinkJob( Configuration Configuration, std::wstring OutputName, Project::Kind Kind, std::span< std::shared_ptr< CompileJob > > DependentJobs );

//////////////////////////////////////////////////////////////////////////

private:

	void Run( void ) override;

//////////////////////////////////////////////////////////////////////////

	Configuration                        m_Configuration;

	std::vector< std::filesystem::path > m_InputFiles;
	std::wstring                         m_OutputName;

	Project::Kind                        m_Kind;

}; // LinkJob
