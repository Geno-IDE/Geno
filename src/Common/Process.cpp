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

#include "Common/Process.h"

#include "Common/Platform/Win32/Win32Error.h"
#include "Common/Platform/Win32/Win32ProcessInfo.h"

#include <chrono>
#include <codecvt>
#include <iostream>
#include <thread>

#include <io.h>

#if defined(_WIN32)
#include <Windows.h>
#endif // _WIN32

Process::Process(std::wstring command_line)
    : command_line_(std::move(command_line)), exit_code_(Run()) {}

int Process::Run(void) {
#if defined(_WIN32)

  STARTUPINFO startup_info = {};
  int fd_in = _fileno(stdin);
  int fd_out = _fileno(stdout);
  int fd_err = _fileno(stderr);
  Win32ProcessInfo process_info;

  startup_info.cb = sizeof(STARTUPINFO);
  startup_info.wShowWindow = SW_HIDE;
  startup_info.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  startup_info.hStdInput = ((fd_in > 0) ? (HANDLE)_get_osfhandle(fd_in)
                                        : GetStdHandle(STD_INPUT_HANDLE));
  startup_info.hStdOutput = ((fd_out > 0) ? (HANDLE)_get_osfhandle(fd_out)
                                          : GetStdHandle(STD_OUTPUT_HANDLE));
  startup_info.hStdError = ((fd_err > 0) ? (HANDLE)_get_osfhandle(fd_err)
                                         : GetStdHandle(STD_ERROR_HANDLE));

  static std::wstring_convert<std::codecvt_utf8<wchar_t>> convert_utf8;
  const std::string command_line_utf8 = convert_utf8.to_bytes(
      command_line_.data(), command_line_.data() + command_line_.size());

  std::cout << "¤ " << command_line_utf8 << "\n";

  if (!WIN32_CALL(CreateProcessW(
          nullptr, const_cast<LPWSTR>(command_line_.data()), nullptr, nullptr,
          TRUE, 0, nullptr, nullptr, &startup_info, &process_info)))
    return -1;

  //////////////////////////////////////////////////////////////////////////

  BOOL result;
  DWORD exit_code;

  while (WIN32_CALL(
             result = GetExitCodeProcess(process_info->hProcess, &exit_code)) &&
         exit_code == STILL_ACTIVE) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return result ? static_cast<int>(exit_code) : -1;

#endif // _WIN32
}
