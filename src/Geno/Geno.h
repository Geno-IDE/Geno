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

// Namespace macros
#define GENO_NAMESPACE       ::Geno::
#define GENO_NAMESPACE_BEGIN namespace Geno {
#define GENO_NAMESPACE_END   }

// Constructor macros
#define GENO_DISABLE_COPY( CLASS )             \
    CLASS( const CLASS& )            = delete; \
    CLASS& operator=( const CLASS& ) = delete;
#define GENO_DISABLE_MOVE( CLASS )        \
    CLASS( CLASS&& )            = delete; \
    CLASS& operator=( CLASS&& ) = delete;
#define GENO_DISABLE_COPY_AND_MOVE( CLASS )    \
    CLASS( const CLASS& )            = delete; \
    CLASS( CLASS&& )                 = delete; \
    CLASS& operator=( const CLASS& ) = delete; \
    CLASS& operator=( CLASS&& )      = delete;
#define GENO_DEFAULT_COPY( CLASS )              \
    CLASS( const CLASS& )            = default; \
    CLASS& operator=( const CLASS& ) = default;
#define GENO_DEFAULT_MOVE( CLASS )         \
    CLASS( CLASS&& )            = default; \
    CLASS& operator=( CLASS&& ) = default;
#define GENO_DEFAULT_COPY_AND_MOVE( CLASS )     \
    CLASS( const CLASS& )            = default; \
    CLASS( CLASS&& )                 = default; \
    CLASS& operator=( const CLASS& ) = default; \
    CLASS& operator=( CLASS&& )      = default;
