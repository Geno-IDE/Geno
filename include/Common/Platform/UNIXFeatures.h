#pragma once

#include <fcntl.h>
#if defined(_WIN32)
#include <string.h>
#include <io.h>
#define dup2 _dup2
#define dup _dup
#define open _open
#define lseek _lseek
#define read _read
#define close _close
#define write _write
#define pipe _pipe
#define fileno _fileno
#define O_RDONLY _O_RDONLY
#else
#include <unistd.h>
#include <stdio.h>
#endif
