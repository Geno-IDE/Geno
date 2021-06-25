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
#include <Common/Macros.h>
#include <Common/Texture2D.h>
#include <filesystem>
#include <string>
#include <vector>

class Drop;
struct ImGuiTabBar;

class TextEdit
{
   public:
	struct Palette
	{
		unsigned int Default;
		unsigned int Keyword;
		unsigned int Number;
		unsigned int String;
		unsigned int Comment;
		unsigned int LineNumber;
		unsigned int Cursor;
		unsigned int Selection;
		unsigned int CurrentLine;
		unsigned int CurrentLineInactive;
		unsigned int CurrentLineEdge;
	};

	struct Glyph
	{
		char         c;
		unsigned int color;

		Glyph( char c, unsigned int color )
		 : c( c )
		 , color( color )
		{
		}
	};

	struct Coordinate
	{
		int x;
		int y;

		Coordinate()
		 : x( 0 )
		 , y( 0 )
		{
		}
		Coordinate( int x, int y )
		 : x( x )
		 , y( y )
		{
		}

		bool operator==( const Coordinate& other ) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=( const Coordinate& other ) const
		{
			return !operator==( other );
		}

		bool operator>( const Coordinate& other ) const
		{
			if( y > other.y ) return true;
			if( y < other.y ) return false;

			return x > other.x;
		}

		bool operator<( const Coordinate& other ) const
		{
			return !operator>( other );
		}

		bool operator>=( const Coordinate& other ) const
		{
			return operator>( other ) || operator==( other );
		}

		bool operator<=( const Coordinate& other ) const
		{
			return operator<( other ) || operator==( other );
		}
	};

	struct Cursor
	{
		Coordinate selectionStart;
		Coordinate selectionEnd;
		Coordinate position;
		Coordinate selectionOrigin = Coordinate( ~0, ~0 );

		bool disabled = false;
	};

	typedef std::vector< Glyph > Line;

	struct File
	{
		std::filesystem::path Path;
		std::string           Text;

		std::vector< Line > Lines;

		bool Open    = true;
		bool Changed = false;

		std::vector< Cursor > cursors;

	}; // File

	//////////////////////////////////////////////////////////////////////////

	TextEdit( void );

	//////////////////////////////////////////////////////////////////////////

	void Show( bool* pOpen );
	void AddFile( const std::filesystem::path& rPath );
	void OnDragDrop( const Drop& rDrop, int X, int Y );

	//////////////////////////////////////////////////////////////////////////

	static float fontSize;

   private:
	typedef Coordinate Scroll;

	struct Properties
	{
		float CharAdvanceY;
		float LineNumMaxWidth;
		float ScrollX;
		float ScrollY;
		bool  Changes;
		int   CursorBlink = 400;

	} props;

	void        SplitLines( File& file );
	bool        RenderEditor( File& file );
	void        HandleKeyboardInputs( File& file );
	void        HandleMouseInputs( File& file );
	void        ScrollToCursor( File& file );
	void        CalculeteLineNumMaxWidth( File& file );
	bool        HasSelection( File& file, int cursor ) const;
	Cursor*     IsCoordinateInSelection( File& file, Coordinate coordinate, int offset = 0 );
	int         IsLineSelected( File& file, int line, Coordinate* start, Coordinate* end ) const;
	float       GetCursorDistance( File& file, int cursor ) const;
	float       GetDistance( File& file, Coordinate position ) const;
	std::string GetWordAt( File& file, Cursor& cursor ) const;
	std::string GetWordAt( File& file, Coordinate position, Coordinate* start, Coordinate* end ) const;
	void        SetSelectionLine( File& file, int line );
	void        SetSelection( File& file, Coordinate start, Coordinate end, int cursor );
	Coordinate  GetMouseCoordinate( File& file, float* distance = nullptr );
	void        AdjustCursors( File& file, int cursor, int xOffset, int yOffset );
	void        YeetDuplicateCursors( File& file );
	void        DisableIntersectingSelections( File& file, int cursor );
	void        DeleteDisabledCursor( File& file );
	void        DeleteSelection( File& file, int cursor );
	void        Enter( File& file );
	void        Backspace( File& file );
	void        Del( File& file );
	void        Tab( File& file, bool shift );
	void        EnterTextStuff( File& file, char c, bool shift = false );
	void        MoveUp( File& file, bool shift );
	void        MoveDown( File& file, bool shift );
	void        MoveRight( File& file, bool ctrl, bool shift );
	void        MoveLeft( File& file, bool ctrl, bool shift );

	Palette palette;

	//////////////////////////////////////////////////////////////////////////

	ImGuiTabBar* m_pTabBar = nullptr;

	Texture2D m_DraggedBitmapTexture = {};

	std::vector< File > m_Files = {};

}; // TextEdit
