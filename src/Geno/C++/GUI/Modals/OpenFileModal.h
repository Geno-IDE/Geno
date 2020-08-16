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

class OpenFileModal : public IModal
{
	GENO_SINGLETON( OpenFileModal );

public:

	using Callback = void( * )( const std::filesystem::path& path, void* user );

public:

	void RequestFile     ( std::string_view title, void* user, Callback callback );
	void RequestDirectory( std::string_view title, void* user, Callback callback );

private:

	std::string PopupID      ( void ) override { return "OpenFile"; }
	std::string Title        ( void ) override { return title_; }
	void        UpdateDerived( void ) override;
	void        OnClose      ( void ) override;

private:

	std::filesystem::path RootDirectory( void );

private:

	std::string               title_;
	std::filesystem::path     current_directory_;
	std::filesystem::path     selected_path_;
	std::filesystem::path     editing_path_;
	Callback                  callback_               = nullptr;
	void*                     user_                   = nullptr;
	bool                      editing_path_is_folder_ = false;
	bool                      change_edit_focus_      = false;
	bool                      directory_requested_    = false;

#if defined( _WIN32 )

	std::unique_ptr< char[] > drives_buffer_;
	size_t                    drives_buffer_size_     = 0;
	size_t                    current_drive_index_    = 0;

#endif // _WIN32

};
