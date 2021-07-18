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
#include "Common/Texture2D.h"
#include "GUI/Modals/IModal.h"

#include <Common/Macros.h>
#include <filesystem>
#include <functional>
#include <map>
#include <vector>

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
	using Callback = std::function< void( const std::filesystem::path& ) >;

//////////////////////////////////////////////////////////////////////////

	void SetCurrentDirectory( std::filesystem::path Directory );
	void Show( std::string Title, const char* pFileFilters, Callback Callback );
	void Show( Callback Callback ); // Used For Opening Folder

//////////////////////////////////////////////////////////////////////////

private:
	std::string PopupID( void ) override { return "OpenFile"; }
	std::string Title( void ) override { return m_Title; }
	void        UpdateDerived( void ) override;
	void        OnClose( void ) override;

//////////////////////////////////////////////////////////////////////////

	std::string                   m_Title;
	std::string                   m_SearchResult;
	std::filesystem::path         m_CurrentPath;
	std::filesystem::path         m_SelectedFile = {};
	std::map< std::string, bool > m_FileFilters;

	Callback m_Callback      = {};
	bool     m_SearchEnabled = false;
	bool     m_OpenFolder    = false;
	bool     m_CreateFolder  = false;

	Texture2D m_IconFolder = {};
	Texture2D m_IconFile   = {};
	Texture2D m_IconSearch = {};

	std::vector< std::string > m_Volumes = {};

}; // OpenFileModal
