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
#include "GUI/Modals/IModal.h"
#include "Common/Texture2D.h"

#include <filesystem>
#include <memory>
#include <functional>
#include <vector>

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

	using Callback = std::function<void(void)>;

//////////////////////////////////////////////////////////////////////////

	void SetCurrentDirectory ( std::filesystem::path Directory );
	void Show                ( std::string Title, std::string FileFilter, Callback Callback );

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
	std::string               m_SearchResult;
	std::filesystem::path     m_CurrentPath;
	std::vector<const char*>  m_FileFilters;

	Callback                  m_Callback            = { };
	bool                      m_SearchEnabled       = false;

	Texture2D                 m_IconFolder          = { };
    Texture2D                 m_IconFile            = { };

#if defined( _WIN32 )

	std::unique_ptr< char[] > m_DrivesBuffer;
	size_t                    m_DrivesBufferSize    = 0;
	size_t                    m_CurrentDriveIndex   = 0;

#endif // _WIN32

}; // OpenFileModal
