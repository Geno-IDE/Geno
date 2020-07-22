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

#include "OutputWindow.h"

#include <iostream>

#include <fcntl.h>
#include <io.h>

#include <imgui.h>

OutputWindow::OutputWindow( void )
{
	if( int out = _fileno( stdout ); out > 0 )
	{
		setvbuf( stdout, nullptr, _IONBF, 0 );
		old_stdout_ = _dup( out );
	}

	if( int err = _fileno( stderr ); err > 0 )
	{
		setvbuf( stderr, nullptr, _IONBF, 0 );
		old_stderr_ = _dup( _fileno( stderr ) );
	}

	if( _pipe( pipe_, 65536, O_BINARY ) != -1 )
	{
		BeginCapture();

		std::cout << "stdout\n";
		std::cerr << "stderr\n";
	}
}

OutputWindow::OutputWindow( OutputWindow&& /*other*/ )
{
}

OutputWindow::~OutputWindow( void )
{
	EndCapture();

	if( old_stdout_ > 0 )
		_close( old_stdout_ );

	if( old_stderr_ > 0 )
		_close( old_stderr_ );

	if( pipe_[ READ ] > 0 )
		_close( pipe_[ READ ] );

	if( pipe_[ WRITE ] > 0 )
		_close( pipe_[ WRITE ] );
}

OutputWindow& OutputWindow::operator=( OutputWindow&& /*other*/ )
{
	return *this;
}

void OutputWindow::Show( void )
{
	if( ImGui::Begin( "Output", &show_ ) )
	{
		EndCapture();
		BeginCapture();

		ImGui::TextUnformatted( captured_.c_str(), captured_.c_str() + captured_.size() );
	}
	ImGui::End();
}

void OutputWindow::BeginCapture( void )
{
	if( int out = _fileno( stdout ); out > 0 )
		_dup2( pipe_[ WRITE ], out );

	if( int err = _fileno( stderr ); err > 0 )
		_dup2( pipe_[ WRITE ], err );
}

void OutputWindow::EndCapture( void )
{
	if( int out = _fileno( stdout ); out > 0 )
		_dup2( old_stdout_, out );

	if( int err = _fileno( stderr ); err > 0 )
		_dup2( old_stderr_, err );

	if( pipe_[ READ ] > 0 )
	{
		char   buf[ 1024 ];
		size_t bytes_read = 0;

		if( !_eof( pipe_[ READ ] ) )
		{
			memset( buf, 0, std::size( buf ) );
			bytes_read = _read( pipe_[ READ ], buf, std::size( buf ) );
			captured_ += buf;
		}

		while( bytes_read == std::size( buf ) )
		{
			bytes_read = 0;

			if( !_eof( pipe_[ READ ] ) )
			{
				memset( buf, 0, std::size( buf ) );
				bytes_read = _read( pipe_[ READ ], buf, std::size( buf ) );
				captured_ += buf;
			}
		}
	}
}
