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

#include "OutputWidget.h"

#include <filesystem>

#include <fcntl.h>
#include <io.h>

#include <imgui.h>

enum
{
	READ,
	WRITE,
};

constexpr uint32_t pipe_size = 64 * 1024;

//////////////////////////////////////////////////////////////////////////

OutputWidget::OutputWidget( void )
{
	RedirectOutputStream( &m_StdOut, stdout );
	RedirectOutputStream( &m_StdErr, stderr );

	// Need stdout and stderr
	GENO_ASSERT( m_StdOut > 0 );
	GENO_ASSERT( m_StdErr > 0 );

	// Make stdout and stderr unbuffered so that we don't need to fflush before and after capture
	GENO_ASSERT( setvbuf( stdout, nullptr, _IONBF, 0 ) == 0 );
	GENO_ASSERT( setvbuf( stderr, nullptr, _IONBF, 0 ) == 0 );

	// Duplicate stdout and stderr
	GENO_ASSERT( ( m_OldStdOut = _dup( m_StdOut ) ) > 0 );
	GENO_ASSERT( ( m_OldStdErr = _dup( m_StdErr ) ) > 0 );

	GENO_ASSERT( _pipe( m_Pipe, pipe_size, O_BINARY ) != -1 );

	// Associate stdout and stderr with the output pipe
	GENO_ASSERT( _dup2( m_Pipe[ WRITE ], m_StdOut ) == 0 );
	GENO_ASSERT( _dup2( m_Pipe[ WRITE ], m_StdErr ) == 0 );

} // OutputWidget

//////////////////////////////////////////////////////////////////////////

OutputWidget::~OutputWidget( void )
{
	GENO_ASSERT( _dup2( m_OldStdOut, m_StdOut ) == 0 );
	GENO_ASSERT( _dup2( m_OldStdErr, m_StdErr ) == 0 );

	if( m_OldStdOut > 0 ) _close( m_OldStdOut );
	if( m_OldStdErr > 0 ) _close( m_OldStdErr );

	if( m_Pipe[ READ ] > 0 )  _close( m_Pipe[ READ ] );
	if( m_Pipe[ WRITE ] > 0 ) _close( m_Pipe[ WRITE ] );

} // ~OutputWidget

//////////////////////////////////////////////////////////////////////////

void OutputWidget::Show( bool* pOpen )
{
	if( ImGui::Begin( "Output", pOpen ) )
	{
		Capture();

		ImGui::TextUnformatted( m_Captured.c_str(), m_Captured.c_str() + m_Captured.size() );

	} ImGui::End();

} // Show

//////////////////////////////////////////////////////////////////////////

void OutputWidget::ClearCapture( void )
{
	m_Captured.clear();

} // ClearCapture

//////////////////////////////////////////////////////////////////////////

void OutputWidget::RedirectOutputStream( int* pFileDescriptor, FILE* pFileStream )
{
	if( ( *pFileDescriptor = _fileno( pFileStream ) ) < 0 )
	{

	#if defined( _WIN32 )

		if( FILE* f; freopen_s( &f, "CONOUT$", "w", pFileStream ) == 0 )
			*pFileDescriptor = _fileno( f );
		else if( freopen_s( &f, "NUL", "w", pFileStream ) == 0 )
			*pFileDescriptor = _fileno( f );

	#else // _WIN32

		if( FILE* f = freopen( "/dev/null", "w", pFileStream ); f != nullptr )
			*pFileDescriptor = _fileno( f );

	#endif // _WIN32

	}

	GENO_ASSERT( *pFileDescriptor > 0 );

} // RedirectOutputStream

//////////////////////////////////////////////////////////////////////////

void OutputWidget::Capture( void )
{
	if( !_eof( m_Pipe[ READ ] ) )
	{
		int64_t StartingOffset = _telli64( m_Pipe[ READ ] );
		size_t  BytesInFront   = ( size_t )_lseeki64( m_Pipe[ READ ], 0, SEEK_END );
		size_t  OldSize        = m_Captured.size();

		_lseeki64( m_Pipe[ READ ], StartingOffset, SEEK_SET );

		m_Captured.resize( OldSize + BytesInFront );

		for( size_t bytes_read = 0; bytes_read < BytesInFront; bytes_read += _read( m_Pipe[ READ ], &m_Captured[ OldSize + bytes_read ], ( uint32_t )( BytesInFront - bytes_read ) ) );
	}

} // Capture
