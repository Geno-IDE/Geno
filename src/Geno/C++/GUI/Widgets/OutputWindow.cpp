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

#include "OutputWindow.h"

#include <filesystem>

#include "Common/Platform/UNIXFeatures.h"

#include <imgui.h>

enum
{
	READ,
	WRITE,
};

constexpr uint32_t pipe_size = 64 * 1024;

//////////////////////////////////////////////////////////////////////////

OutputWindow::OutputWindow( void )
{
	RedirectOutputStream( &m_StdOut, stdout );
	RedirectOutputStream( &m_StdErr, stderr );

	#if !defined(_WIN32)
	m_Captured = NULL;
	#endif

	// Need stdout and stderr
	GENO_ASSERT( m_StdOut > 0 );
	GENO_ASSERT( m_StdErr > 0 );

	// Make stdout and stderr unbuffered so that we don't need to fflush before and after capture
	GENO_ASSERT( setvbuf( stdout, nullptr, _IONBF, 0 ) == 0 );
	GENO_ASSERT( setvbuf( stderr, nullptr, _IONBF, 0 ) == 0 );

	// Duplicate stdout and stderr
	GENO_ASSERT( ( m_OldStdOut = dup( m_StdOut ) ) > 0 );
	GENO_ASSERT( ( m_OldStdErr = dup( m_StdErr ) ) > 0 );

	GENO_ASSERT( pipe( m_Pipe ) != -1 );

	// Associate stdout and stderr with the output pipe
	GENO_ASSERT( dup2( m_Pipe[ WRITE ], m_StdOut ) == 0 );
	GENO_ASSERT( dup2( m_Pipe[ WRITE ], m_StdErr ) == 0 );
} // OutputWidget

//////////////////////////////////////////////////////////////////////////

OutputWindow::~OutputWindow( void )
{
	GENO_ASSERT( dup2( m_OldStdOut, m_StdOut ) == 0 );
	GENO_ASSERT( dup2( m_OldStdErr, m_StdErr ) == 0 );

	if( m_OldStdOut > 0 ) close( m_OldStdOut );
	if( m_OldStdErr > 0 ) close( m_OldStdErr );

	if( m_Pipe[ READ ] > 0 )  close( m_Pipe[ READ ] );
	if( m_Pipe[ WRITE ] > 0 ) close( m_Pipe[ WRITE ] );
} // ~OutputWidget

//////////////////////////////////////////////////////////////////////////

void OutputWindow::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350*2, 196 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Output", pOpen ) )
	{
		Capture();

		ImGui::TextUnformatted( m_Captured,  m_Captured);

	} ImGui::End();

} // Show

//////////////////////////////////////////////////////////////////////////

void OutputWindow::ClearCapture( void )
{
	#if defined(_WIN32)
	m_Captured.clear()
	#else
	free(m_Captured);
	#endif
} // ClearCapture

//////////////////////////////////////////////////////////////////////////

void OutputWindow::RedirectOutputStream( int* pFileDescriptor, FILE* pFileStream )
{
	if( ( *pFileDescriptor = fileno( pFileStream ) ) < 0 )
	{
	#if defined( _WIN32 )
		if( FILE* f; freopen_s( &f, "CONOUT$", "w", pFileStream ) == 0 )
			*pFileDescriptor = fileno( f );
		else if( freopen_s( &f, "NUL", "w", pFileStream ) == 0 )
			*pFileDescriptor = fileno( f );
	#else // _WIN32
		if( FILE* f = freopen( "/dev/null", "w", pFileStream ); f != nullptr )
			*pFileDescriptor = fileno( f );
	#endif // _WIN32
	}


	GENO_ASSERT( *pFileDescriptor > 0 );

} // RedirectOutputStream

//////////////////////////////////////////////////////////////////////////
void OutputWindow::Capture( void )
{
	int64_t StartingOffset = lseek( m_Pipe[ READ ], 0, SEEK_CUR );
	size_t  BytesInFront   = ( size_t )lseek( m_Pipe[ READ ], 0, SEEK_END );
	#if defined(_WIN32)
	size_t OldSize = m_Captured.size();
	#else
	size_t  OldSize        = m_Captured ? strlen(m_Captured) : 0;
	#endif

	lseek( m_Pipe[ READ ], StartingOffset, SEEK_SET );

	#if defined(_WIN32)
	m_Captured.resize(OldSize + BytesInFront);
	#else
	m_Captured = static_cast<char*> (realloc(m_Captured, OldSize + BytesInFront));
	#endif

	for( size_t bytes_read = 0; bytes_read < BytesInFront; bytes_read += read( m_Pipe[ READ ], &m_Captured[ OldSize + bytes_read ], ( uint32_t )( BytesInFront - bytes_read ) ) );
} // Capture
