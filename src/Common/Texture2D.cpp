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

#include "Common/Texture2D.h"

//////////////////////////////////////////////////////////////////////////

Texture2D::~Texture2D( void )
{
	if( m_ID )
		glDeleteTextures( 1, &m_ID );

} // ~Texture2D

//////////////////////////////////////////////////////////////////////////

void Texture2D::SetPixels( GLint InternalFormat, GLsizei Width, GLsizei Height, GLenum Format, const GLvoid* pData )
{
	if( !m_ID )
		CreateTexture();

	glBindTexture( GL_TEXTURE_2D, m_ID );
	glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, pData );
	glGenerateMipmap( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );

} // SetPixels

//////////////////////////////////////////////////////////////////////////

void Texture2D::CreateTexture( void )
{
	glCreateTextures( GL_TEXTURE_2D, 1, &m_ID );
	glBindTexture( GL_TEXTURE_2D, m_ID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glBindTexture( GL_TEXTURE_2D, 0 );

} // CreateTexture
