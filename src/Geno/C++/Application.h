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
#include "Components/Workspace.h"

#include <optional>
#include <vector>

#include <Common/Macros.h>

class IModal;

class Application
{
	GENO_SINGLETON( Application );

	 Application( void ) = default;
	~Application( void );

//////////////////////////////////////////////////////////////////////////

public:

	int        Run             ( int NumArgs, char** ppArgs );
	void       NewWorkspace    ( std::filesystem::path Location, std::string Name );
	bool       LoadWorkspace   ( std::filesystem::path Path );
	void       CloseWorkspace  ( void );
	void       PushModal       ( IModal* pModal );
	void       PopModal        ( void );
	IModal*    NextModal       ( IModal* pPrevious );
	Workspace* CurrentWorkspace( void );

//////////////////////////////////////////////////////////////////////////

private:

	void HandleCommandLineArgs( int NumArgs, char** ppArgs );

//////////////////////////////////////////////////////////////////////////

	std::optional< Workspace > m_CurrentWorkspace;
	std::vector< IModal* >     m_ModalStack;
	std::filesystem::path      m_Location;

}; // Application
