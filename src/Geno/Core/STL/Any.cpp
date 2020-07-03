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

#include "Any.h"

GENO_NAMESPACE_BEGIN

Any::Any( void )
	: storage_  { }
	, ctor_func_{ nullptr }
	, dtor_func_{ nullptr }
	, move_func_{ nullptr }
{
}

Any::Any( Any&& other )
	: storage_  { }
	, ctor_func_{ other.ctor_func_ }
	, dtor_func_{ other.dtor_func_ }
	, move_func_{ other.move_func_ }
{
	( this->*ctor_func_ )( &other.storage_ );
}

Any::~Any( void )
{
	if( dtor_func_ )
		( this->*dtor_func_)();
}

Any& Any::operator=( Any&& other )
{
	ctor_func_ = other.ctor_func_;
	dtor_func_ = other.dtor_func_;
	move_func_ = other.move_func_;

	( this->*move_func_ )( std::move( other ) );

	return *this;
}

GENO_NAMESPACE_END
