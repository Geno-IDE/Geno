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
#include <filesystem>

namespace GCL
{

class Object;

class Serializer
{
public:
	
	explicit Serializer( const std::filesystem::path& rPath );
		    ~Serializer( void );

//////////////////////////////////////////////////////////////////////////

	void WriteObject( const Object& rObject, int IndentLevel = 0 );

//////////////////////////////////////////////////////////////////////////

	bool IsOpen( void ) const;

//////////////////////////////////////////////////////////////////////////

private:

	int m_FileDescriptor = -1;

}; // Serializer

}
