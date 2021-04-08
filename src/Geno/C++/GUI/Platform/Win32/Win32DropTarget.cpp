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

#if defined( _WIN32 )

#include "GUI/Platform/Win32/Win32DropTarget.h"

#include "GUI/MainWindow.h"

#include <Common/Platform/Win32/Win32Error.h>
#include <Common/Drop.h>

#include <iostream>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <ShlObj.h>

//////////////////////////////////////////////////////////////////////////

static constexpr short FormatPriority( const FORMATETC& rFormat )
{
	switch( rFormat.cfFormat )
	{
		default:             return 0;
		// Bitmap
		case CF_DIBV5:       return 10;
		case CF_DIB:         return 9;
		case CF_TIFF:        return 8;
		case CF_BITMAP:      return 7;
		// Audio
		case CF_RIFF:        return 6;
		case CF_WAVE:        return 5;
		// Text
		case CF_UNICODETEXT: return 4;
		case CF_OEMTEXT:     return 3;
		case CF_TEXT:        return 2;
		// Paths
		case CF_HDROP:       return 1;
	}

} // FormatPriority

//////////////////////////////////////////////////////////////////////////

static constexpr bool PrioritySortFormatsPred( const FORMATETC& rA, const FORMATETC& rB )
{
	return FormatPriority( rA ) > FormatPriority( rB );

} // PrioritySortFormatsPred

//////////////////////////////////////////////////////////////////////////

Win32DropTarget::Win32DropTarget( void )
{
	HWND WindowHandle = glfwGetWin32Window( glfwGetCurrentContext() );

	WIN32_CALL( OleInitialize( nullptr ) );
	WIN32_CALL( CoLockObjectExternal( this, true, false ) );
	WIN32_CALL( RegisterDragDrop( WindowHandle, this ) );

} // Win32DropTarget

//////////////////////////////////////////////////////////////////////////

Win32DropTarget::~Win32DropTarget( void )
{
	HWND WindowHandle = glfwGetWin32Window( glfwGetCurrentContext() );

	WIN32_CALL( CoLockObjectExternal( this, false, false ) );
	WIN32_CALL( RevokeDragDrop( WindowHandle ) );
	OleUninitialize();

} // ~Win32DropTarget

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Win32DropTarget::QueryInterface( const IID& rIID, void** ppObject )
{
	if( rIID == IID_IUnknown || rIID == IID_IDropTarget )
	{
		*ppObject = this;
		AddRef();

		return S_OK;
	}
	else
	{
		*ppObject = NULL;

		return E_NOINTERFACE;
	}

} // QueryInterface

//////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE Win32DropTarget::AddRef( void )
{
	return InterlockedIncrement( &m_RefCount );

} // AddRef

//////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE Win32DropTarget::Release( void )
{
	const LONG Ref = InterlockedDecrement( &m_RefCount );
	if( Ref == 0 )
		delete this;

	return Ref;

} // Release

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragEnter( IDataObject* pDataObject, DWORD /*KeyState*/, POINTL Point, DWORD* /*pEffect*/ )
{
	::Drop Drop;
	if( DropFromDataObject( pDataObject, Drop ) )
	{
		MainWindow::Instance().DragEnter( std::move( Drop ), Point.x, Point.y );
	}

	return S_OK;

} // DragEnter

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragOver( DWORD /*KeyState*/, POINTL Point, DWORD* /*pEffect*/ )
{
	MainWindow::Instance().DragOver( Point.x, Point.y );

	return S_OK;

} // DragOver

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragLeave( void )
{
	MainWindow::Instance().DragLeave();

	return S_OK;

} // DragLeave

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Win32DropTarget::Drop( IDataObject* pDataObject, DWORD /*KeyState*/, POINTL Point, DWORD* /*pEffect*/ )
{
	::Drop Drop;
	if( DropFromDataObject( pDataObject, Drop ) )
	{
		MainWindow::Instance().DragDrop( Drop, Point.x, Point.y );
	}

	return S_OK;

} // Drop

//////////////////////////////////////////////////////////////////////////

bool Win32DropTarget::DropFromDataObject( IDataObject* pDataObject, ::Drop& rOutDrop )
{
	std::vector< FORMATETC > SupportedFormats;
	IEnumFORMATETC*          pFormatEnumerator;
	if( pDataObject->EnumFormatEtc( DATADIR_GET, &pFormatEnumerator ) == S_OK )
	{
		FORMATETC Format;
		while( pFormatEnumerator->Next( 1, &Format, nullptr ) == S_OK )
		{
			if( Format.cfFormat < CF_MAX )
				SupportedFormats.push_back( Format );
		}

		std::sort( SupportedFormats.begin(), SupportedFormats.end(), PrioritySortFormatsPred );
	}

	for( FORMATETC& rFormat : SupportedFormats )
	{
		if( pDataObject->QueryGetData( &rFormat ) == S_OK )
		{
			// STGMEDIUM with garbage collector
			struct CSTGMEDIUM : STGMEDIUM { ~CSTGMEDIUM() { ReleaseStgMedium( this ); } };

			CSTGMEDIUM Medium{ };
			if( pDataObject->GetData( &rFormat, &Medium ) == S_OK )
			{
				switch( Medium.tymed )
				{
					case TYMED_HGLOBAL:
					{
						void* pGlobalData = GlobalLock( Medium.hGlobal );
						// Unlock HGLOBAL at the end of scope
						struct GC { HGLOBAL GlobalHandle; ~GC( void ) { GlobalUnlock( GlobalHandle ); } } gc{ Medium.hGlobal };

						switch( rFormat.cfFormat )
						{
							case CF_DIBV5:
							{
								BITMAPV5HEADER* pBitmapV5Header  = static_cast< LPBITMAPV5HEADER >( pGlobalData );
								HCOLORSPACE     ColorSpaceHandle = reinterpret_cast< HCOLORSPACE >( pBitmapV5Header + 1 );
								BYTE*           pBits            = reinterpret_cast< LPBYTE >( ColorSpaceHandle + 1 );
								Drop::Bitmap    Bitmap;
								Bitmap.width                     = pBitmapV5Header->bV5Width;
								Bitmap.height                    = pBitmapV5Header->bV5Height;
								Bitmap.data                      = std::make_unique< uint8_t[] >( pBitmapV5Header->bV5SizeImage );

								std::memcpy( Bitmap.data.get(), pBits, pBitmapV5Header->bV5SizeImage );

								rOutDrop.SetBitmap( std::move( Bitmap ) );

								return true;
							}

							case CF_DIB:
							{
								BITMAPINFO*  pBitmapInfo = static_cast< LPBITMAPINFO >( pGlobalData );
								BYTE*        pBits       = reinterpret_cast< LPBYTE >( pBitmapInfo + 1 );
								Drop::Bitmap Bitmap;
								Bitmap.width             = pBitmapInfo->bmiHeader.biWidth;
								Bitmap.height            = pBitmapInfo->bmiHeader.biHeight;
								Bitmap.data              = std::make_unique< uint8_t[] >( pBitmapInfo->bmiHeader.biSizeImage );

								std::memcpy( Bitmap.data.get(), pBits, pBitmapInfo->bmiHeader.biSizeImage );

								rOutDrop.SetBitmap( std::move( Bitmap ) );

								return true;
							}

							case CF_UNICODETEXT:
							{
								Drop::Text Text = static_cast< LPCWSTR >( pGlobalData );
								rOutDrop.SetText( std::move( Text ) );

								return true;
							}

							case CF_OEMTEXT:
							{
								const char* pChars = static_cast< const char* >( pGlobalData );
								int         Length = static_cast< int >( strlen( pChars ) );
								Drop::Text  Text( Length, L'\0' );
								MultiByteToWideChar( CP_OEMCP, 0, pChars, Length, Text.data(), Length );

								rOutDrop.SetText( std::move( Text ) );

								return true;
							}

							case CF_TEXT:
							{
								const char* pChars = static_cast< const char* >( pGlobalData );
								int         Length = static_cast< int >( strlen( pChars ) );
								Drop::Text  Text( Length, L'\0' );
								MultiByteToWideChar( CP_ACP, 0, pChars, Length, Text.data(), Length );

								rOutDrop.SetText( std::move( Text ) );

								return true;
							}

							case CF_HDROP:
							{
								HDROP       DropHandle = static_cast< HDROP >( pGlobalData );
								UINT        DropCount  = DragQueryFileW( DropHandle, 0xFFFFFFFF, nullptr, 0 );
								Drop::Paths Paths;

								for( UINT i = 0; i < DropCount; ++i )
								{
									const UINT   Length = DragQueryFileW( DropHandle, i, nullptr, 0 );
									std::wstring Path( Length, '\0' );
									DragQueryFileW( DropHandle, i, Path.data(), static_cast< UINT >( Path.size() ) + 1 );

									Paths.emplace_back( std::move( Path ) );
								}

								rOutDrop.SetPaths( std::move( Paths ) );

								return true;
							}
						}

					} break;

					default:
					{
						std::cerr << "Unsupported medium type for drop\n";

					} break;
				}
			}
		}
	}

	return false;

} // DropFromDataObject

#endif // _WIN32
