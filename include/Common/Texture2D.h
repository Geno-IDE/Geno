/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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
#include <GL/glew.h>

#include <cstdint>

class Texture2D
{
public:

	~Texture2D( void );

//////////////////////////////////////////////////////////////////////////

	void SetPixels( GLint InternalFormat, GLsizei Width, GLsizei Height, GLenum Format, const GLvoid* pData );

//////////////////////////////////////////////////////////////////////////

	GLuint   GetID         ( void ) const { return m_ID; }
	uint16_t GetWidth      ( void ) const { return m_Width; }
	uint16_t GetHeight     ( void ) const { return m_Height; }
	float    GetAspectRatio( void ) const { return static_cast< float >( m_Width ) / m_Height; }

//////////////////////////////////////////////////////////////////////////

private:

	void CreateTexture( void );

//////////////////////////////////////////////////////////////////////////

	GLuint   m_ID     = 0;

	uint16_t m_Width  = 0;
	uint16_t m_Height = 0;

}; // Texture2D
