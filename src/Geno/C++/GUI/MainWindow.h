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
#include <Common/Drop.h>
#include <Common/Macros.h>

#include <optional>
#include <string>

#include <imgui.h>

class  MainMenuBar;
class  OutputWindow;
class  TextEdit;
class  Win32DropTarget;
class  WorkspaceOutliner;
struct GLFWwindow;
struct ImGuiContext;
struct ImGuiSettingsHandler;
struct ImFont;

class MainWindow
{
	GENO_SINGLETON( MainWindow );

	 MainWindow( void );
	~MainWindow( void );

//////////////////////////////////////////////////////////////////////////

public:

	void MakeCurrent         ( void );
	bool BeginFrame          ( void );
	void EndFrame            ( void );
	void PushHorizontalLayout( void );
	void PopHorizontalLayout ( void );
	void DragEnter           ( Drop Drop, int X, int Y );
	void DragOver            ( int X, int Y );
	void DragLeave           ( void );
	void DragDrop            ( const Drop& rDrop, int X, int Y );

//////////////////////////////////////////////////////////////////////////

	const Drop* GetDraggedDrop( void ) const { return m_DraggedDrop.has_value() ? &m_DraggedDrop.value() : nullptr; }
	ImFont*     GetFontSans   ( void ) const { return m_pFontSans; }
	ImFont*     GetFontMono   ( void ) const { return m_pFontMono; }
	int         GetDragPosX   ( void ) const { return m_DragPosX; }
	int         GetDragPosY   ( void ) const { return m_DragPosY; }

//////////////////////////////////////////////////////////////////////////

	MainMenuBar*       pMenuBar           = nullptr;
	WorkspaceOutliner* pWorkspaceOutliner = nullptr;
	TextEdit*          pTextEdit          = nullptr;
	OutputWindow*      pOutputWindow      = nullptr;

//////////////////////////////////////////////////////////////////////////

private:

	static void  GLFWErrorCB            ( int Error, const char* pDescription );
	static void  GLFWSizeCB             ( GLFWwindow* pWindow, int Width, int Height );
	static void* ImGuiSettingsReadOpenCB( ImGuiContext* pContext, ImGuiSettingsHandler* pHandler, const char* pName );
	static void  ImGuiSettingsReadLineCB( ImGuiContext* pContext, ImGuiSettingsHandler* pHandler, void* pEntry, const char* pLine );
	static void  ImGuiSettingsWriteAllCB( ImGuiContext* pContext, ImGuiSettingsHandler* pHandler, ImGuiTextBuffer* pOutBuffer );

//////////////////////////////////////////////////////////////////////////

	std::filesystem::path m_IniPath            = { };

	std::optional< Drop > m_DraggedDrop        = { };

	GLFWwindow*           m_pWindow            = nullptr;
	ImGuiContext*         m_pImGuiContext      = nullptr;
	ImFont*               m_pFontSans          = nullptr;
	ImFont*               m_pFontMono          = nullptr;

	int                   m_Width              = 0;
	int                   m_Height             = 0;
	int                   m_LayoutStackCounter = 0;
	int                   m_DragPosX           = 0;
	int                   m_DragPosY           = 0;

#if defined( _WIN32 )
	Win32DropTarget*      m_pDropTarget        = nullptr;
#endif // _WIN32

}; // MainWindow
