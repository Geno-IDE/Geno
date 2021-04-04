/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product,
 * an acknowledgment in the product documentation would be appreciated but is
 * not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "GUI/Platform/Win32/Win32DropTarget.h"

#include "GUI/MainWindow.h"

#include <Common/Drop.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <ShlObj.h>

static short FormatPriority(const FORMATETC &format) {
  switch (format.cfFormat) {
  default:
    return 0;
  // Bitmap
  case CF_DIBV5:
    return 10;
  case CF_DIB:
    return 9;
  case CF_TIFF:
    return 8;
  case CF_BITMAP:
    return 7;
  // Audio
  case CF_RIFF:
    return 6;
  case CF_WAVE:
    return 5;
  // Text
  case CF_UNICODETEXT:
    return 4;
  case CF_OEMTEXT:
    return 3;
  case CF_TEXT:
    return 2;
  // Paths
  case CF_HDROP:
    return 1;
  }
}

static bool PrioritySortFormatsPred(const FORMATETC &a, const FORMATETC &b) {
  return FormatPriority(a) > FormatPriority(b);
}

Win32DropTarget::Win32DropTarget(void) {
  HWND hwnd = glfwGetWin32Window(glfwGetCurrentContext());

  OleInitialize(nullptr);
  CoLockObjectExternal(this, true, false);
  RegisterDragDrop(hwnd, this);
}

Win32DropTarget::~Win32DropTarget(void) {
  HWND hwnd = glfwGetWin32Window(glfwGetCurrentContext());

  CoLockObjectExternal(this, false, false);
  RevokeDragDrop(hwnd);
  OleUninitialize();
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::QueryInterface(REFIID riid,
                                                          void **object) {
  if (riid == IID_IUnknown || riid == IID_IDropTarget) {
    *object = this;
    AddRef();

    return S_OK;
  }

  *object = NULL;

  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32DropTarget::AddRef(void) {
  return InterlockedIncrement(&ref_count_);
}

ULONG STDMETHODCALLTYPE Win32DropTarget::Release(void) {
  const LONG ref = InterlockedDecrement(&ref_count_);
  if (ref == 0)
    delete this;

  return ref;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragEnter(IDataObject *data_obj,
                                                     DWORD /*key_state*/,
                                                     POINTL point,
                                                     DWORD * /*effect*/) {
  ::Drop drop;
  if (DropFromDataObject(data_obj, drop)) {
    MainWindow::Instance().DragEnter(std::move(drop), point.x, point.y);
  }

  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragOver(DWORD /*key_state*/,
                                                    POINTL point,
                                                    DWORD * /*effect*/) {
  MainWindow::Instance().DragOver(point.x, point.y);

  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::DragLeave(void) {
  MainWindow::Instance().DragLeave();

  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32DropTarget::Drop(IDataObject *data_obj,
                                                DWORD /*key_state*/,
                                                POINTL point,
                                                DWORD * /*effect*/) {
  ::Drop drop;
  if (DropFromDataObject(data_obj, drop)) {
    MainWindow::Instance().DragDrop(drop, point.x, point.y);
  }

  return S_OK;
}

bool Win32DropTarget::DropFromDataObject(IDataObject *data_obj,
                                         ::Drop &out_drop) {
  std::vector<FORMATETC> supported_formats;
  IEnumFORMATETC *format_enumerator;
  if (data_obj->EnumFormatEtc(DATADIR_GET, &format_enumerator) == S_OK) {
    FORMATETC format;
    while (format_enumerator->Next(1, &format, nullptr) == S_OK) {
      if (format.cfFormat < CF_MAX)
        supported_formats.push_back(format);
    }

    std::sort(supported_formats.begin(), supported_formats.end(),
              PrioritySortFormatsPred);
  }

  for (FORMATETC &format : supported_formats) {
    if (data_obj->QueryGetData(&format) == S_OK) {
      // STGMEDIUM with garbage collector
      struct CSTGMEDIUM : STGMEDIUM {
        ~CSTGMEDIUM() { ReleaseStgMedium(this); }
      };

      CSTGMEDIUM medium{};
      if (data_obj->GetData(&format, &medium) == S_OK) {
        switch (medium.tymed) {
        case TYMED_HGLOBAL: {
          LPVOID global_data = GlobalLock(medium.hGlobal);
          // Unlock HGLOBAL at the end of scope
          struct GC {
            HGLOBAL hglobal;
            ~GC() { GlobalUnlock(hglobal); }
          } gc{medium.hGlobal};

          switch (format.cfFormat) {
          case CF_DIBV5: {
            LPBITMAPV5HEADER bitmap_v5_header =
                static_cast<LPBITMAPV5HEADER>(global_data);
            HCOLORSPACE colorspace =
                reinterpret_cast<HCOLORSPACE>(bitmap_v5_header + 1);
            LPBYTE bits = reinterpret_cast<LPBYTE>(colorspace + 1);
            Drop::Bitmap bitmap;
            bitmap.width = bitmap_v5_header->bV5Width;
            bitmap.height = bitmap_v5_header->bV5Height;
            bitmap.data =
                std::make_unique<uint8_t[]>(bitmap_v5_header->bV5SizeImage);

            std::memcpy(bitmap.data.get(), bits,
                        bitmap_v5_header->bV5SizeImage);

            out_drop.SetBitmap(std::move(bitmap));

            return true;
          }

          case CF_DIB: {
            LPBITMAPINFO bitmap_info = static_cast<LPBITMAPINFO>(global_data);
            LPBYTE bits = reinterpret_cast<LPBYTE>(bitmap_info + 1);
            Drop::Bitmap bitmap;
            bitmap.width = bitmap_info->bmiHeader.biWidth;
            bitmap.height = bitmap_info->bmiHeader.biHeight;
            bitmap.data =
                std::make_unique<uint8_t[]>(bitmap_info->bmiHeader.biSizeImage);

            std::memcpy(bitmap.data.get(), bits,
                        bitmap_info->bmiHeader.biSizeImage);

            out_drop.SetBitmap(std::move(bitmap));

            return true;
          }

          case CF_UNICODETEXT: {
            Drop::Text text = static_cast<LPCWSTR>(global_data);
            out_drop.SetText(std::move(text));

            return true;
          }

          case CF_OEMTEXT: {
            LPSTR chars = static_cast<LPSTR>(global_data);
            size_t length = strlen(chars);
            Drop::Text text = Drop::Text(length, L'\0');
            MultiByteToWideChar(CP_OEMCP, 0, chars, static_cast<int>(length),
                                text.data(), static_cast<int>(length));

            out_drop.SetText(std::move(text));

            return true;
          }

          case CF_TEXT: {
            LPSTR chars = static_cast<LPSTR>(global_data);
            size_t length = strlen(chars);
            Drop::Text text = Drop::Text(length, L'\0');
            MultiByteToWideChar(CP_ACP, 0, chars, static_cast<int>(length),
                                text.data(), static_cast<int>(length));

            out_drop.SetText(std::move(text));

            return true;
          }

          case CF_HDROP: {
            HDROP hdrop = static_cast<HDROP>(global_data);
            UINT count = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
            Drop::Paths paths;

            for (UINT i = 0; i < count; ++i) {
              const UINT length = DragQueryFileW(hdrop, i, nullptr, 0);
              std::wstring path = std::wstring(length, '\0');
              DragQueryFileW(hdrop, i, path.data(),
                             static_cast<UINT>(path.size()) + 1);

              paths.emplace_back(std::move(path));
            }

            out_drop.SetPaths(std::move(paths));

            return true;
          }
          }
        }
        }
      }
    }
  }

  return false;
}
