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
#include "Components/BuildMatrix.h"

#include <string_view>
#include <vector>

#include <Common/Macros.h>
#include <Common/Texture2D.h>

class  Drop;
struct CompilationDone;
struct WorkspaceOpened;

class TitleBar
{
public:

#if defined ( __linux__ )

	struct Point
	{
	public:
		float x, y;

		Point() : x( 0.0F ), y( 0.0F ) { }
		Point( float _x, float _y ) : x( _x ), y( _y ) { }
	};

	/*
	*				 <-- L -->
	*
	*
	*
	*
	* A					P					D
	* |||||||||||||||||||||||||||||||||||||||
	* |										|
	* |										|
	* |||||||||||||||||||||||||||||||||||||||
	* B					Q					C
	*
	*/

	struct Rect
	{
	public:
		float x1;
		float y1;
		float x2;
		float y2;

		Rect()
		{
			x1  = y1 = 0;
			x2 = y2 = -1;
		}

		Rect( float _x1, float _y1, float _x2, float _y2 )
		{
			x1  = _x1;
			y1  = _y1;
			x2  = _x2;
			y2  = _y2;
		}

		Rect( Point& top_left, float width, float height )
		{
			x1  = top_left.x;
			y1  = top_left.y;
			x2  = top_left.x + width - 1;
			y2  = top_left.y + height - 1;
		}

		Point BottomLeft () { return Point( x1, y2 ); }
		Point BottomRight() { return Point( x2, y2 ); }
		Point TopLeft    () { return Point( x1, y1 ); }
		Point TopRight   () { return Point( x2, y1 ); }

		int Top    () { return y1; }
		int Right  () { return x2; }
		int Left   () { return x1; }
		int Bottom () { return y2; }

};
#endif
//////////////////////////////////////////////////////////////////////////

	TitleBar( void );

//////////////////////////////////////////////////////////////////////////

	void Draw( void );

//////////////////////////////////////////////////////////////////////////

	float Height( void ) const { return m_Height; }

//////////////////////////////////////////////////////////////////////////

	bool ShowTextEdit            = false;
	bool ShowDemoWindow          = false;
	bool ShowAboutWindow         = false;
	bool ShowOutputWindow        = false;
	bool ShowWorkspaceOutliner   = false;
	bool ShowGenoDiscordSettings = false;

//////////////////////////////////////////////////////////////////////////

private:

	void ActionFileNewWorkspace    ( void );
	void ActionExtShowGenoDiscord  ( void );
	void ActionFileOpenWorkspace   ( void );
	void ActionFileCloseWorkspace  ( void );
	void ActionBuildBuildAndRun    ( void );
	void ActionBuildBuild          ( void );
	void AddBuildMatrixColumn      ( BuildMatrix::Column& rColumn );

//////////////////////////////////////////////////////////////////////////

	Texture2D m_IconTexture = { };

	float     m_Height      = 0.0f;

}; // TitleBar
