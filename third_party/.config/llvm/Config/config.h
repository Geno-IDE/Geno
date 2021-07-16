#ifndef CONFIG_H
#define CONFIG_H

// Include this header only under the llvm source tree.
// This is a private header.

// Exported configuration
#include "llvm/Config/llvm-config.h"

// No bug report URL
#define BUG_REPORT_URL "https://bugs.llvm.org/"

// Enable backtraces
#define ENABLE_BACKTRACES 1

// Enable crash overrides
#define ENABLE_CRASH_OVERRIDES 1

// Enable crash memory dumps
#define LLVM_ENABLE_CRASH_DUMPS 1

// Check if the `backtrace` function is available
#define BACKTRACE_HEADER <execinfo.h>
#if __has_include( BACKTRACE_HEADER )
#define HAVE_BACKTRACE 1
#endif // __has_include( BACKTRACE_HEADER )

// Check if we can use __crashreporter_info__
#if defined( __APPLE__ )
#define HAVE_CRASHREPORTERCLIENT_H 1
#define HAVE_CRASHREPORTER_INFO    1
#endif // __APPLE__

// Don't use `arc4random`
#define HAVE_DECL_ARC4RANDOM 0

// Check if we have <fenv.h>
#if __has_include( <fenv.h> )
#define HAVE_DECL_FE_ALL_EXCEPT 1
#define HAVE_DECL_FE_INEXACT HAVE_DECL_FE_ALL_EXCEPT
#endif // __has_include( <fenv.h> )

// Check if we have strerror_s
#if defined( _WIN32 )
#define HAVE_DECL_STRERROR_S 1
#endif // _WIN32

// We don't have the DIA SDK installed
#define LLVM_ENABLE_DIA_SDK 0

// Check if we have the <dlfcn.h> header file
#if __has_include( <dlfcn.h> )
#define HAVE_DLFCN_H 1
#endif // __has_include( <dlfcn.h> )

// Check if dlopen() and dladdr() are available on this platform
#if defined( __unix__ ) || defined( __unix )
#define HAVE_DLOPEN 
#define HAVE_DLADDR HAVE_DLADDR
#endif // __unix__ || __unix

// Check if we can register EH frames on this platform
#if defined( __linux__ )
#define HAVE_REGISTER_FRAME   1
#define HAVE_DEREGISTER_FRAME 1
#endif // __linux__

// Check if we have the <errno.h> header file
#if __has_include( <errno.h> )
#define HAVE_ERRNO_H 1
#endif // __has_include( <errno.h> )

// Check if we have the <fcntl.h> header file
#if __has_include( <fcntl.h> )
#define HAVE_FCNTL_H 1
#endif // __has_include( <fcntl.h> )

// Check if we have the <fenv.h> header file
#if __has_include( <fenv.h> )
#define HAVE_FENV_H 1
#endif // __has_include( <fenv.h> )

// Check if we have the <ffi/ffi.h> header file
#if __has_include( <ffi/ffi.h> )
#define HAVE_FFI_FFI_H 1
#endif // __has_include( <ffi/ffi.h> )

// Check if we have the <ffi.h> header file
#if __has_include( <ffi.h> )
#define HAVE_FFI_H 1
#endif // __has_include( <ffi.h> )

// Check if libffi is available on this platform
#if defined( HAVE_FFI_FFI_H ) && defined( HAVE_FFI_H )
#define HAVE_FFI_CALL 1
#endif // HAVE_FFI_FFI_H && HAVE_FFI_H

// Check if we have the `futimens` function
#define HAVE_FUTIMENS 1

// Check if we have the `futimes` function
#define HAVE_FUTIMES 1

// Check if we have the `getpagesize` function
#define HAVE_GETPAGESIZE 1

// Check if we have the `getrlimit` function
#define HAVE_GETRLIMIT 1

// Check if we have the `getrusage` function
#define HAVE_GETRUSAGE 1

// Check if we have the `isatty` function
#define HAVE_ISATTY 1

// Check if we have the `edit` library (-ledit)
#define HAVE_LIBEDIT 1

// Check if we have the `pfm` library (-lpfm)
#define HAVE_LIBPFM 1

// `perf_branch_entry` struct has field cycles
#define LIBPFM_HAS_FIELD_CYCLES 1

// Check if we have the `psapi` library (-lpsapi)
#define HAVE_LIBPSAPI 1

// Check if we have the `pthread` library (-lpthread)
#define HAVE_LIBPTHREAD 1

// Check if we have the `pthread_getname_np` function
#define HAVE_PTHREAD_GETNAME_NP 1

// Check if we have the `pthread_setname_np` function
#define HAVE_PTHREAD_SETNAME_NP 1

// Check if we have the <link.h> header file
#if __has_include( <link.h> )
#define HAVE_LINK_H 1
#endif // __has_include( <link.h> )

// Check if we have the `lseek64` function
#define HAVE_LSEEK64 1

// Check if we have the <mach/mach.h> header file
#if __has_include( <mach/mach.h> )
#define HAVE_MACH_MACH_H 1
#endif // __has_include( <mach/mach.h> )

// Check if we have the `mallctl` function
#if __has_include( <malloc_np.h> )
#define HAVE_MALLCTL 1
#endif // __has_include( <malloc_np.h> )

// Check if we have the `mallinfo` function
#define HAVE_MALLINFO 1

// Check if we have the <malloc/malloc.h> header file
#if __has_include( <malloc/malloc.h> )
#define HAVE_MALLOC_MALLOC_H 1
#endif // __has_include( <malloc/malloc.h> )

// Check if we have the `malloc_zone_statistics` function
#define HAVE_MALLOC_ZONE_STATISTICS 1

// Check if we have the `posix_fallocate` function
#define HAVE_POSIX_FALLOCATE 1

// Check if we have the `posix_spawn` function
#define HAVE_POSIX_SPAWN 1

// Check if we have the `pread` function
#define HAVE_PREAD 1

// Have pthread_getspecific
#define HAVE_PTHREAD_GETSPECIFIC 1

// Check if we have the <pthread.h> header file
#if __has_include( <pthread.h> )
#define HAVE_PTHREAD_H 1
#endif // __has_include( <pthread.h> )

// Have pthread_mutex_lock
#define HAVE_PTHREAD_MUTEX_LOCK 1

// Have pthread_rwlock_init
#define HAVE_PTHREAD_RWLOCK_INIT 1

// Check if we have the `sbrk` function
#define HAVE_SBRK 1

// Check if we have the `setenv` function
#define HAVE_SETENV 1

// Check if we have the `setrlimit` function
#define HAVE_SETRLIMIT 1

// Check if we have the `sigaltstack` function
#define HAVE_SIGALTSTACK 1

// Check if we have the <signal.h> header file
#if __has_include( <signal.h> )
#define HAVE_SIGNAL_H 1
#endif // __has_include( <signal.h> )

// Check if we have the `strerror` function
#define HAVE_STRERROR 1

// Check if we have the `strerror_r` function
#if !defined( _WIN32 )
#define HAVE_STRERROR_R 1
#endif // _WIN32

// Check if we have the `sysconf` function
#define HAVE_SYSCONF 1

// Check if we have the <sys/ioctl.h> header file
#if __has_include( <sys/ioctl.h> )
#define HAVE_SYS_IOCTL_H 1
#endif // __has_include( <sys/ioctl.h> )

// Check if we have the <sys/mman.h> header file
#if __has_include( <sys/mman.h> )
#define HAVE_SYS_MMAN_H 1
#endif // __has_include( <sys/mman.h> )

// Check if we have the <sys/param.h> header file
#if __has_include( <sys/param.h> )
#define HAVE_SYS_PARAM_H 1
#endif // __has_include( <sys/param.h> )

// Check if we have the <sys/resource.h> header file
#if __has_include( <sys/resource.h> )
#define HAVE_SYS_RESOURCE_H 1
#endif // __has_include( <sys/resource.h> )

// Check if we have the <sys/stat.h> header file
#if __has_include( <sys/stat.h> )
#define HAVE_SYS_STAT_H 1
#endif // __has_include( <sys/stat.h> )

// Check if we have the <sys/time.h> header file
#if __has_include( <sys/time.h> )
#define HAVE_SYS_TIME_H 1
#endif // __has_include( <sys/time.h> )

#if defined( _WIN32 )
	// stat struct has st_mtimespec member
	#define HAVE_STRUCT_STAT_ST_MTIMESPEC_TV_NSEC 1
#else // _WIN32
	// stat struct has st_mtim member
	#define HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC 1
#endif // !_WIN32

// Check if we have the <sys/types.h> header file
#if __has_include( <sys/types.h> )
#define HAVE_SYS_TYPES_H 1
#endif // __has_include( <sys/types.h> )

// the setupterm() function is supported on this platform
#define LLVM_ENABLE_TERMINFO 1

// the xar_open() function is supported on this platform
#define HAVE_LIBXAR 1

// Check if we have the <termios.h> header file
#if __has_include( <termios.h> )
#define HAVE_TERMIOS_H 1
#endif // __has_include( <termios.h> )

// Check if we have the <unistd.h> header file
#if __has_include( <unistd.h> )
#define HAVE_UNISTD_H 1
#endif // __has_include( <unistd.h> )

// Check if we have the <valgrind/valgrind.h> header file
#if __has_include( <valgrind/valgrind.h> )
#define HAVE_VALGRIND_VALGRIND_H 1
#endif // __has_include( <valgrind/valgrind.h> )

// Check if we have the `_chsize_s` function
#define HAVE__CHSIZE_S 1

// Check if we have the `_Unwind_Backtrace` function
#define HAVE__UNWIND_BACKTRACE 1

// Have host's _alloca
// Check if libgcc functions are available
#if defined( __unix__ ) || defined( __unix )
#define HAVE__ALLOCA      1
#define HAVE___ALLOCA     1
#define HAVE___ASHLDI3    1
#define HAVE___ASHRDI3    1
#define HAVE___CHKSTK     1
#define HAVE___CHKSTK_MS  1
#define HAVE___CMPDI2     1
#define HAVE___DIVDI3     1
#define HAVE___FIXDFDI    1
#define HAVE___FIXSFDI    1
#define HAVE___FLOATDIDF  1
#define HAVE___LSHRDI3    1
#define HAVE___MAIN       1
#define HAVE___MODDI3     1
#define HAVE___UDIVDI3    1
#define HAVE___UMODDI3    1
#define HAVE____CHKSTK    1
#define HAVE____CHKSTK_MS 1
#endif // __unix__ || __unix

// Linker version detected at compile time
//#define HOST_LINK_VERSION

// Target triple LLVM will generate code for by default
#define LLVM_DEFAULT_TARGET_TRIPLE LLVM_HOST_TRIPLE

// zlib compression is available
#define LLVM_ENABLE_ZLIB __has_include( <zlib.h> )

// overriding target triple is disabled
//#define LLVM_TARGET_TRIPLE_ENV "${LLVM_TARGET_TRIPLE_ENV}"

// LLVM version information
#define LLVM_VERSION_INFO 12.0

// tools show host and target info when invoked with --version
#define LLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO 1

// libxml2 is supported on this platform
#define LLVM_ENABLE_LIBXML2 1

// the extension used for shared libraries
#if defined( _WIN32 )
#define LTDL_SHLIB_EXT ".dll"
#else // _WIN32
#define LTDL_SHLIB_EXT ".so"
#endif // !_WIN32

// the address where bug reports for this package should be sent
//#define PACKAGE_BUGREPORT

// the full name of this package
#define PACKAGE_NAME "llvm-geno-integration"

// the version of this package
#define PACKAGE_VERSION LLVM_VERSION_STRING

// the full name and version of this package
#define PACKAGE_STRING PACKAGE_NAME PACKAGE_VERSION

// the vendor of this package
//#define PACKAGE_VENDOR

// the return type of signal handlers
#define RETSIGTYPE void

// std::is_trivially_copyable is supported
#define HAVE_STD_IS_TRIVIALLY_COPYABLE 1

// a function implementing stricmp
//#define stricmp

// a function implementing strdup
//#define strdup

// GlobalISel rule coverage is being collected
#define LLVM_GISEL_COV_ENABLED 1

// the default GlobalISel coverage file prefix
//#define LLVM_GISEL_COV_PREFIX

// Timers signpost passes in Xcode Instruments
#if defined( __APPLE__ )
#define LLVM_SUPPORT_XCODE_SIGNPOSTS 1
#endif // __APPLE__

#endif // CONFIG_H
