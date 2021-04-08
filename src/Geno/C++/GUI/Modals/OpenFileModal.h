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
#include "GUI/Modals/IModal.h"

#include <filesystem>
#include <memory>

#include <Common/Macros.h>

#if defined( _WIN32 )
#include <Windows.h>
#undef SetCurrentDirectory
#endif // _WIN32

class OpenFileModal : public IModal
{
	GENO_SINGLETON( OpenFileModal );

	OpenFileModal( void );

//////////////////////////////////////////////////////////////////////////

public:

	using Callback = void( * )( const std::filesystem::path& path, void* user );

//////////////////////////////////////////////////////////////////////////

	void SetCurrentDirectory ( std::filesystem::path Directory );
	void RequestFile         ( std::string Title, void* pUser, Callback Callback );
	void RequestDirectory    ( std::string Title, void* pUser, Callback Callback );

//////////////////////////////////////////////////////////////////////////

private:

	std::string PopupID      ( void ) override { return "OpenFile"; }
	std::string Title        ( void ) override { return m_Title; }
	void        UpdateDerived( void ) override;
	void        OnClose      ( void ) override;

//////////////////////////////////////////////////////////////////////////

	std::filesystem::path RootDirectory( void );

//////////////////////////////////////////////////////////////////////////

	std::string               m_Title;
	std::filesystem::path     m_CurrentDirectory;
	std::filesystem::path     m_SelectedPath;
	std::filesystem::path     m_EditingPath;

	Callback                  m_Callback            = nullptr;
	void*                     m_pUser               = nullptr;
	bool                      m_EditingPathIsFolder = false;
	bool                      m_ChangeEditFocus     = false;
	bool                      m_DirectoryRequested  = false;

#if defined( _WIN32 )

	std::unique_ptr< char[] > m_DrivesBuffer;
	size_t                    m_DrivesBufferSize    = 0;
	size_t                    m_CurrentDriveIndex   = 0;

#endif // _WIN32

}; // OpenFileModal
