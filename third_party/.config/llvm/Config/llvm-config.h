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

// Host triple LLVM will be executed on
#define LLVM_HOST_TRIPLE "unknown-unknown-unknown"

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

#endif // LLVM_CONFIG_H
