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
#include "Common/Macros.h"

#include <filesystem>
#include <string>
#include <vector>

#include <imgui.h>

class  Win32DropTarget;
struct GLFWwindow;
struct ImGuiContext;

class MainWindow
{
public:

	GENO_SINGLETON( MainWindow );

	~MainWindow( void );

public:

	void Init                ( void );
	void MakeCurrent         ( void );
	bool BeginFrame          ( void );
	void EndFrame            ( void );
	void PushHorizontalLayout( void );
	void PopHorizontalLayout ( void );
	void DragEnter           ( std::wstring_view file_path, int x, int y );
	void DragOver            ( int x, int y );
	void DragLeave           ( void );
	void DragDrop            ( std::wstring_view file_path, int x, int y );

public:

	bool HasDraggedFiles( void ) const { return !dragged_files_.empty(); }
	int  GetDragPosX    ( void ) const { return drag_pos_x_; }
	int  GetDragPosY    ( void ) const { return drag_pos_y_; }

private:

	static void GLFWSizeCB( GLFWwindow* window, int width, int height );

private:

	GLFWwindow*   window_         = nullptr;
	ImGuiContext* im_gui_context_ = nullptr;

	std::vector< std::filesystem::path > dragged_files_;
	std::string                          ini_path_;

	int width_                = 0;
	int height_               = 0;
	int layout_stack_counter_ = 0;
	int drag_pos_x_           = 0;
	int drag_pos_y_           = 0;

#if defined( _WIN32 )
	Win32DropTarget* drop_target_ = nullptr;
#endif // _WIN32

};
