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
		unsigned int x;
		unsigned int y;

		Coordinate()
		 : x( 0 )
		 , y( 0 )
		{
		}
		Coordinate( unsigned int x, unsigned int y )
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

		bool Open = true;

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
		float        charAdvanceY;
		float        lineNumMaxWidth;
		Scroll       scroll;
		bool         changes;
		unsigned int cursorBlink = 400;

	} props;

	void        SplitLines( File& file );
	bool        RenderEditor( File& file );
	void        HandleKeyboardInputs( File& file );
	void        HandleMouseInputs( File& file );
	void        CalculeteLineNumMaxWidth( File& file );
	bool        HasSelection( File& file, unsigned int cursor ) const;
	bool        IsCoordinateInSelection( File& file, Coordinate coordinate, bool includePosition );
	bool        IsLineSelected( File& file, unsigned int line, Coordinate* start, Coordinate* end ) const;
	float       GetCursorDistance( File& file, unsigned int cursor ) const;
	float       GetDistance( File& file, Coordinate position ) const;
	std::string GetWordAt( File& file, Cursor& cursor ) const;
	std::string GetWordAt( File& file, Coordinate position, Coordinate* start, Coordinate* end ) const;
	void        SetSelectionLine( File& file, unsigned int line );
	void        SetSelection( File& file, Coordinate start, Coordinate end, unsigned int cursor );
	Coordinate  GetMouseCoordinate( File& file, float* distance = nullptr );
	void        AdjustCursors( File& file, unsigned int cursor, unsigned int xOffset, unsigned int yOffset );
	void        YeetDuplicateCursors( File& file );
	void        DisableIntersectingSelections( File& file, unsigned int cursor );
	void        Enter( File& file, unsigned int cursor );
	void        Backspace( File& file, unsigned int cursor );
	void        EnterTextStuff( File& file, char c );
	void        MoveUp( File& file );
	void        MoveDown( File& file );
	void        MoveRight( File& file );
	void        MoveLeft( File& file );

	Palette palette;

	//////////////////////////////////////////////////////////////////////////

	ImGuiTabBar* m_pTabBar = nullptr;

	Texture2D m_DraggedBitmapTexture = {};

	std::vector< File > m_Files = {};

}; // TextEdit
