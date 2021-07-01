/*===------- llvm/Config/abi-breaking.h - llvm configuration -------*- C -*-===*/
/*                                                                            */
/* Part of the LLVM Project, under the Apache License v2.0 with LLVM          */
/* Exceptions.                                                                */
/* See https://llvm.org/LICENSE.txt for license information.                  */
/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

/* This file controls the C++ ABI break introduced in LLVM public header. */

#ifndef LLVM_ABI_BREAKING_CHECKS_H
#define LLVM_ABI_BREAKING_CHECKS_H

// Enable checks that alter the LLVM C++ ABI
#define LLVM_ENABLE_ABI_BREAKING_CHECKS 1

// Enable reverse iteration of unordered llvm containers
#define LLVM_ENABLE_REVERSE_ITERATION 1

// Allow selectively disabling link-time mismatch checking so that header-only ADT content from LLVM can be used without linking libSupport
#if !defined( LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING ) || !LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING

// ABI_BREAKING_CHECKS protection: provides link-time failure when clients build mismatch with LLVM
#if defined( _MSC_VER )

// Use pragma with MSVC
#define LLVM_XSTR( S ) LLVM_STR( S )
#define LLVM_STR( S ) #S
#pragma detect_mismatch( "LLVM_ENABLE_ABI_BREAKING_CHECKS", LLVM_XSTR( LLVM_ENABLE_ABI_BREAKING_CHECKS ) )
#undef LLVM_XSTR
#undef LLVM_STR

#elif defined( _WIN32 ) || defined( __CYGWIN__ ) // _MSC_VER

// FIXME: Implement checks without weak.
#elif defined( __cplusplus )
#if !( defined( _AIX ) && defined( __GNUC__ ) && !defined( __clang__ ) )
#define LLVM_HIDDEN_VISIBILITY __attribute__( ( visibility( "hidden" ) ) )
#else // !( _AIX && __GNUC__ && ! __clang__ )
// GCC on AIX does not support visibility attributes. Symbols are not exported by default on AIX
#define LLVM_HIDDEN_VISIBILITY
#endif // _AIX && __GNUC__ && ! __clang__

namespace llvm
{

#if LLVM_ENABLE_ABI_BREAKING_CHECKS

extern int EnableABIBreakingChecks;
LLVM_HIDDEN_VISIBILITY __attribute__( ( weak ) ) int* VerifyEnableABIBreakingChecks = &EnableABIBreakingChecks;

#else // LLVM_ENABLE_ABI_BREAKING_CHECKS

extern int DisableABIBreakingChecks;
LLVM_HIDDEN_VISIBILITY __attribute__( ( weak ) ) int *VerifyDisableABIBreakingChecks = &DisableABIBreakingChecks;

#endif // !LLVM_ENABLE_ABI_BREAKING_CHECKS

}

#undef LLVM_HIDDEN_VISIBILITY
#endif // _MSC_VER

#endif // LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING

#endif // LLVM_ABI_BREAKING_CHECKS_H
