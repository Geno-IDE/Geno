/*===------- llvm/Config/llvm-config.h - llvm configuration -------*- C -*-===*/
/*                                                                            */
/* Part of the LLVM Project, under the Apache License v2.0 with LLVM          */
/* Exceptions.                                                                */
/* See https://llvm.org/LICENSE.txt for license information.                  */
/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

/* This file enumerates variables from the LLVM configuration so that they
   can be in exported headers and won't override package specific directives.
   This is a C header that can be included in the llvm-c headers. */

#ifndef LLVM_CONFIG_H
#define LLVM_CONFIG_H

// Enable threads
#define LLVM_ENABLE_THREADS 1

// Has atomic intrinsics
#define LLVM_HAS_ATOMICS 1

// Host architecture
#if defined( __aarch64__ )
#define LLVM_HOST_ARCH "aarch64"
#elif defined( __arm__ ) || defined( _M_ARM ) // __aarch64__
#define LLVM_HOST_ARCH "arm"
#elif defined( __mips__ ) // __arm__ || _M_ARM
#define LLVM_HOST_ARCH "mips"
#elif defined( __powerpc__ ) || defined( _M_PPC ) // __mips__
#define LLVM_HOST_ARCH "ppc"
#elif defined( __x86_64__ ) || defined( _M_X64 ) // __powerpc__ || _M_PPC
#define LLVM_HOST_ARCH "x86_64"
#elif defined( __i386__ ) || defined( _M_IX86 ) // __x86_64__ || _M_X64
#define LLVM_HOST_ARCH "x86"
#endif // __i386__ || _M_IX86

// Host vendor
#if defined( __APPLE__ )
#define LLVM_HOST_VENDOR "Apple"
#else // __APPLE__
#define LLVM_HOST_VENDOR "unknown"
#endif // __APPLE__

// Host operating system
#if defined( _WIN32 )
#define LLVM_HOST_OS "Win32"
#elif defined( __linux__ ) // _WIN32
#define LLVM_HOST_OS "Linux"
#elif defined( __darwin__ ) // __linux__
#define LLVM_HOST_OS "Darwin"
#endif // __darwin__

// Host triple LLVM will be executed on
#define LLVM_HOST_TRIPLE LLVM_HOST_ARCH "-" LLVM_HOST_VENDOR "-" LLVM_HOST_OS

// Check if Unixish platform
#if defined( __unix__ ) || defined( __unix )
#define LLVM_ON_UNIX
#endif // __unix__ || __unix

// LLVM version
#define LLVM_VERSION_MAJOR 12
#define LLVM_VERSION_MINOR 0
#define LLVM_VERSION_PATCH 0
#define LLVM_VERSION_STRING "12.0.0"

// Check if <sysexits.h> is available
#define HAVE_SYSEXITS_H __has_include( <sysexits.h> )

// Enable the experimental new pass manager by default
#define LLVM_ENABLE_NEW_PASS_MANAGER 1

#endif // LLVM_CONFIG_H
