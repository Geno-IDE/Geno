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

#include <fcntl.h>
#include <imgui.h>

#if defined( _WIN32 )
#include <io.h>
#define pipe( Pipe ) _pipe( Pipe, 64 * 1024, O_BINARY )
#elif defined( __unix__ ) || defined( __APPLE__ ) // _WIN32
#include <unistd.h>
#endif // __unix__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

enum
{
	READ,
	WRITE,
};

//////////////////////////////////////////////////////////////////////////

OutputWindow::OutputWindow( void )
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
	GENO_ASSERT( ( m_OldStdOut = dup( m_StdOut ) ) > 0 );
	GENO_ASSERT( ( m_OldStdErr = dup( m_StdErr ) ) > 0 );

	GENO_ASSERT( pipe( m_Pipe ) != -1 );

	// Associate stdout and stderr with the output pipe
	GENO_ASSERT( dup2( m_Pipe[ WRITE ], m_StdOut ) >= 0 );
	GENO_ASSERT( dup2( m_Pipe[ WRITE ], m_StdErr ) >= 0 );

	// Make reading operations non-blocking
	fcntl( m_Pipe[ READ ], F_SETFL, O_NONBLOCK );

} // OutputWidget

//////////////////////////////////////////////////////////////////////////

OutputWindow::~OutputWindow( void )
{
	GENO_ASSERT( dup2( m_OldStdOut, m_StdOut ) >= 0 );
	GENO_ASSERT( dup2( m_OldStdErr, m_StdErr ) >= 0 );

	if( m_OldStdOut > 0 ) close( m_OldStdOut );
	if( m_OldStdErr > 0 ) close( m_OldStdErr );

	if( m_Pipe[ READ  ] > 0 ) close( m_Pipe[ READ ] );
	if( m_Pipe[ WRITE ] > 0 ) close( m_Pipe[ WRITE ] );

} // ~OutputWidget

//////////////////////////////////////////////////////////////////////////

void OutputWindow::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350 * 2, 196 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Output", pOpen ) )
	{
		Capture();

		if( m_pCaptured )
			ImGui::TextUnformatted( m_pCaptured, m_pCaptured + m_CapturedSize );

	} ImGui::End();

} // Show

//////////////////////////////////////////////////////////////////////////

void OutputWindow::ClearCapture( void )
{
	free( m_pCaptured );

	m_pCaptured    = nullptr;
	m_CapturedSize = 0;

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
	char    Buffer[ 1024 ];
	ssize_t BytesRead;

	while( ( BytesRead = read( m_Pipe[ READ ], Buffer, std::size( Buffer ) ) ) > 0 )
	{
		m_pCaptured = static_cast< char* >( realloc( m_pCaptured, m_CapturedSize + BytesRead ) );
		memcpy( &m_pCaptured[ m_CapturedSize ], Buffer, BytesRead );
		m_CapturedSize += BytesRead;
	}

} // Capture
