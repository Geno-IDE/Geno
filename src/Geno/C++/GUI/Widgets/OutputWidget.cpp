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

#include "OutputWidget.h"

#include <cassert>
#include <filesystem>

#include <fcntl.h>
#include <io.h>

#include <imgui.h>

enum
{
	READ,
	WRITE,
};

constexpr uint32_t pipe_size = 65536;

OutputWidget::OutputWidget( void )
{
	RedirectOutputStream( &stdout_, stdout );
	RedirectOutputStream( &stderr_, stderr );

	// Need stdout and stderr
	assert( stdout_ > 0 );
	assert( stderr_ > 0 );

	// Make stdout and stderr unbuffered so that we don't need to fflush before and after capture
	GENO_ASSERT( setvbuf( stdout, nullptr, _IONBF, 0 ) == 0 );
	GENO_ASSERT( setvbuf( stderr, nullptr, _IONBF, 0 ) == 0 );

	// Duplicate stdout and stderr
	GENO_ASSERT( ( old_stdout_ = _dup( stdout_ ) ) > 0 );
	GENO_ASSERT( ( old_stderr_ = _dup( stderr_ ) ) > 0 );

	GENO_ASSERT( _pipe( pipe_, pipe_size, O_BINARY ) != -1 );

	// Associate stdout and stderr with the output pipe
	GENO_ASSERT( _dup2( pipe_[ WRITE ], stdout_ ) == 0 );
	GENO_ASSERT( _dup2( pipe_[ WRITE ], stderr_ ) == 0 );
}

OutputWidget::~OutputWidget( void )
{
	GENO_ASSERT( _dup2( old_stdout_, stdout_ ) == 0 );
	GENO_ASSERT( _dup2( old_stderr_, stderr_ ) == 0 );

	if( old_stdout_ > 0 ) _close( old_stdout_ );
	if( old_stderr_ > 0 ) _close( old_stderr_ );

	if( pipe_[ READ ] > 0 )  _close( pipe_[ READ ] );
	if( pipe_[ WRITE ] > 0 ) _close( pipe_[ WRITE ] );
}

void OutputWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Output", p_open ) )
	{
		Capture();

		ImGui::TextUnformatted( captured_.c_str(), captured_.c_str() + captured_.size() );
	}
	ImGui::End();
}

void OutputWidget::ClearCapture( void )
{
	captured_.clear();
}

OutputWidget& OutputWidget::Instance( void )
{
	static OutputWidget instance;
	return instance;
}

void OutputWidget::RedirectOutputStream( int* fd, FILE* stream )
{
	if( ( *fd = _fileno( stream ) ) < 0 )
	{
	#if defined( _WIN32 )

		if( FILE* f; freopen_s( &f, "CONOUT$", "w", stream ) == 0 )
			*fd = _fileno( f );
		else if( freopen_s( &f, "NUL", "w", stream ) == 0 )
			*fd = _fileno( f );

	#else // _WIN32

		if( FILE* f = freopen( "/dev/null", "w", stream ); f != nullptr )
			*fd = _fileno( f );

	#endif // else
	}

	GENO_ASSERT( *fd > 0 );
}

void OutputWidget::Capture( void )
{
	if( !_eof( pipe_[ READ ] ) )
	{
		int64_t starting_offset = _telli64( pipe_[ READ ] );
		size_t  bytes_in_front  = ( size_t )_lseeki64( pipe_[ READ ], 0, SEEK_END );
		size_t  old_size        = captured_.size();

		_lseeki64( pipe_[ READ ], starting_offset, SEEK_SET );

		captured_.resize( old_size + bytes_in_front );

		for( size_t bytes_read = 0;
		     bytes_read < bytes_in_front;
			 bytes_read += _read( pipe_[ READ ], &captured_[ old_size + bytes_read ], ( uint32_t )( bytes_in_front - bytes_read ) )
		);
	}
}
