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

#include "GUI/Platform/Win32/Win32DropTarget.h"

#include "GUI/MainWindow.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <ShlObj.h>

Win32DropTarget::Win32DropTarget( void )
{
	GLFWwindow* window = glfwGetCurrentContext();
	HWND        hwnd   = glfwGetWin32Window( window );

	OleInitialize( nullptr );
	CoLockObjectExternal( this, true, false );
	RegisterDragDrop( hwnd, this );
}

Win32DropTarget::~Win32DropTarget( void )
{
	GLFWwindow* window = glfwGetCurrentContext();
	HWND        hwnd   = glfwGetWin32Window( window );

	CoLockObjectExternal( this, false, false );
	RevokeDragDrop( hwnd );
	OleUninitialize();
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::QueryInterface( REFIID riid, void** object )
{
	if( riid == IID_IUnknown || riid == IID_IDropTarget )
	{
		*object = this;
		AddRef();

		return S_OK;
	}

	*object = NULL;

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32DropTarget::AddRef( void )
{
	return InterlockedIncrement( &ref_count_ );
}

ULONG STDMETHODCALLTYPE Win32DropTarget::Release( void )
{
	const LONG ref = InterlockedDecrement( &ref_count_ );
	if( ref == 0 )
		delete this;

	return ref;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragEnter( IDataObject* data_obj, DWORD /*key_state*/, POINTL point, DWORD* /*effect*/ )
{
	FORMATETC formatetc;
	formatetc.cfFormat = CF_HDROP;
	formatetc.ptd      = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex   = -1;
	formatetc.tymed    = TYMED_HGLOBAL;

	if( data_obj->QueryGetData( &formatetc ) == S_OK )
	{
		STGMEDIUM stgmedium;

		if( data_obj->GetData( &formatetc, &stgmedium ) == S_OK )
		{
			HDROP drop  = static_cast< HDROP >( GlobalLock( stgmedium.hGlobal ) );
			UINT  count = DragQueryFileW( drop, 0xFFFFFFFF, nullptr, 0 );

			for( UINT i = 0; i < count; ++i )
			{
				const UINT   length = DragQueryFileW( drop, i, nullptr, 0 );
				std::wstring path   = std::wstring( length, '\0' );
				DragQueryFileW( drop, i, path.data(), static_cast< UINT >( path.size() ) + 1 );

				MainWindow::Instance().DragEnter( path, point.x, point.y );
			}

			GlobalUnlock( stgmedium.hGlobal );
			ReleaseStgMedium( &stgmedium );
		}
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragOver( DWORD /*key_state*/, POINTL point, DWORD* /*effect*/ )
{
	MainWindow::Instance().DragOver( point.x, point.y );

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragLeave( void )
{
	MainWindow::Instance().DragLeave();

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::Drop( IDataObject* data_obj, DWORD /*key_state*/, POINTL point, DWORD* /*effect*/ )
{
	FORMATETC formatetc;
	formatetc.cfFormat = CF_HDROP;
	formatetc.ptd      = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex   = -1;
	formatetc.tymed    = TYMED_HGLOBAL;

	if( data_obj->QueryGetData( &formatetc ) == S_OK )
	{
		STGMEDIUM stgmedium;

		if( data_obj->GetData( &formatetc, &stgmedium ) == S_OK )
		{
			HDROP drop  = static_cast< HDROP >( GlobalLock( stgmedium.hGlobal ) );
			UINT  count = DragQueryFileW( drop, 0xFFFFFFFF, nullptr, 0 );

			for( UINT i = 0; i < count; ++i )
			{
				const UINT   length = DragQueryFileW( drop, i, nullptr, 0 );
				std::wstring path   = std::wstring( length, '\0' );
				DragQueryFileW( drop, i, path.data(), static_cast< UINT >( path.size() ) + 1 );

				MainWindow::Instance().DragDrop( path, point.x, point.y );
			}

			GlobalUnlock( stgmedium.hGlobal );
			ReleaseStgMedium( &stgmedium );
		}
	}

	return S_OK;
}
