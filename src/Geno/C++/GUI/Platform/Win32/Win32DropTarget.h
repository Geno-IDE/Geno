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
#if defined( _WIN32 )

#include <Common/Drop.h>
#include <Common/Macros.h>

#include <Windows.h>

class Win32DropTarget : public IDropTarget
{
	GENO_DISABLE_COPY_AND_MOVE( Win32DropTarget );

//////////////////////////////////////////////////////////////////////////

public:

	 Win32DropTarget( void );
	~Win32DropTarget( void );

//////////////////////////////////////////////////////////////////////////

	HRESULT STDMETHODCALLTYPE QueryInterface( const IID& rIID, void** ppObject ) override;
	ULONG   STDMETHODCALLTYPE AddRef        ( void ) override;
	ULONG   STDMETHODCALLTYPE Release       ( void ) override;
	HRESULT STDMETHODCALLTYPE DragEnter     ( IDataObject* pDataObject, DWORD KeyState, POINTL Point, DWORD* pEffect ) override;
	HRESULT STDMETHODCALLTYPE DragOver      ( DWORD KeyState, POINTL Point, DWORD* pEffect ) override;
	HRESULT STDMETHODCALLTYPE DragLeave     ( void ) override;
	HRESULT STDMETHODCALLTYPE Drop          ( IDataObject* pDataObject, DWORD KeyState, POINTL Point, DWORD* pEffect ) override;

//////////////////////////////////////////////////////////////////////////

private:

	bool DropFromDataObject( IDataObject* pDataObject, ::Drop& rOutDrop );

//////////////////////////////////////////////////////////////////////////

	LONG m_RefCount = 1;

}; // Win32DropTarget

#endif // _WIN32
