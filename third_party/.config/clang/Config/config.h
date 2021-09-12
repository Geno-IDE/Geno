#ifdef CLANG_CONFIG_H
#error config.h can only be included once
#else // CLANG_CONFIG_H
#define CLANG_CONFIG_H

// Bug report URL
#define BUG_REPORT_URL "https://bugs.llvm.org/"

// Default linker to use
#define CLANG_DEFAULT_LINKER ""

// Default C/ObjC standard to use
#define CLANG_DEFAULT_STD_C LangStandard::lang_c99

// Default C++/ObjC++ standard to use
#define CLANG_DEFAULT_STD_CXX LangStandard::lang_cxx17

// Default C++ stdlib to use
#define CLANG_DEFAULT_CXX_STDLIB ""

// Default runtime library to use
#define CLANG_DEFAULT_RTLIB ""

// Default unwind library to use
#define CLANG_DEFAULT_UNWINDLIB ""

// Default objcopy to use
#define CLANG_DEFAULT_OBJCOPY ""

// Default OpenMP runtime used by -fopenmp
#define CLANG_DEFAULT_OPENMP_RUNTIME ""

// Default architecture for OpenMP offloading to Nvidia GPUs
#define CLANG_OPENMP_NVPTX_DEFAULT_ARCH ""

// Default architecture for SystemZ
#define CLANG_SYSTEMZ_DEFAULT_ARCH ""

// Multilib suffix for libdir
#define CLANG_LIBDIR_SUFFIX ""

// Relative directory for resource files
#define CLANG_RESOURCE_DIR ""

// Directories clang will search for headers
#define C_INCLUDE_DIRS ""

// Directories clang will search for configuration files
#define CLANG_CONFIG_FILE_SYSTEM_DIR ""
#define CLANG_CONFIG_FILE_USER_DIR ""

// Default <path> to all compiler invocations for --sysroot=<path>
#define DEFAULT_SYSROOT ""

// Directory where gcc is installed
#define GCC_INSTALL_PREFIX ""

// Define if we have libxml2
//#define CLANG_HAVE_LIBXML 1

// Define if we have sys/resource.h (rlimits)
//#define CLANG_HAVE_RLIMITS 1

// The LLVM product name and version
#define BACKEND_PACKAGE_STRING "llvm-geno-integration"

// Linker version detected at compile time
//#define HOST_LINK_VERSION ""

// pass --build-id to ld
//#define ENABLE_LINKER_BUILD_ID

// enable x86 relax relocations by default
#define ENABLE_X86_RELAX_RELOCATIONS 1

// Enable each functionality of modules
#define CLANG_ENABLE_ARCMT           1
#define CLANG_ENABLE_OBJC_REWRITER   1
#define CLANG_ENABLE_STATIC_ANALYZER 1

// Spawn a new process clang.exe for the CC1 tool invocation, when necessary
#define CLANG_SPAWN_CC1 1

#endif // CLANG_CONFIG_H
