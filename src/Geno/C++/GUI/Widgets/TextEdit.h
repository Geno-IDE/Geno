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

#include <imgui.h>

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
		char         C;
		unsigned int Color;

		Glyph( char C, unsigned int Color )
		 : C( C )
		 , Color( Color )
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

		bool operator==( const Coordinate& Other ) const
		{
			return x == Other.x && y == Other.y;
		}

		bool operator!=( const Coordinate& Other ) const
		{
			return !operator==( Other );
		}

		bool operator>( const Coordinate& Other ) const
		{
			if( y > Other.y ) return true;
			if( y < Other.y ) return false;

			return x > Other.x;
		}

		bool operator<( const Coordinate& Other ) const
		{
			return !operator>( Other );
		}

		bool operator>=( const Coordinate& Other ) const
		{
			return operator>( Other ) || operator==( Other );
		}

		bool operator<=( const Coordinate& Other ) const
		{
			return operator<( Other ) || operator==( Other );
		}
	};

	struct Cursor
	{
		Coordinate SelectionStart;
		Coordinate SelectionEnd;
		Coordinate Position;
		Coordinate SelectionOrigin = Coordinate( ~0, ~0 );

		bool Disabled = false;
	};

	typedef std::vector< Glyph > Line;

	struct File
	{
		std::filesystem::path Path;
		std::string           Text;

		std::vector< Line > Lines;

		bool Open    = true;
		bool Changed = false;

		std::vector< Cursor > Cursors;

	}; // File

	//////////////////////////////////////////////////////////////////////////

	TextEdit( void );

	//////////////////////////////////////////////////////////////////////////

	void Show( bool* pOpen );
	void AddFile( const std::filesystem::path& rPath );
	void OnDragDrop( const Drop& rDrop, int X, int Y );
	void SaveFile( File& rFile );
	void SplitLines( File& rFile );
	void JoinLines( File& rFile );

	//////////////////////////////////////////////////////////////////////////

	static float FontSize;

   private:
	typedef Coordinate Scroll;

	struct Properties
	{
		float CharAdvanceY;
		float LineNumMaxWidth;
		float SpaceSize;
		float ScrollX;
		float ScrollY;
		bool  Changes;
		int   CursorBlink = 400;
	} Props;

	bool        RenderEditor( File& rFile );
	void        HandleKeyboardInputs( File& rFile );
	void        HandleMouseInputs( File& rFile );
	void        ScrollToCursor( File& rFile );
	void        CalculeteLineNumMaxWidth( File& rFile );
	bool        HasSelection( File& rFile, int cursor ) const;
	Cursor*     IsCoordinateInSelection( File& rFile, Coordinate Coordinate, int Offset = 0 );
	int         IsLineSelected( File& rFile, int line, Coordinate* pStart, Coordinate* pEnd ) const;
	float       GetCursorDistance( File& rFile, int Cursor ) const;
	float       GetDistance( File& rFile, Coordinate Position ) const;
	std::string GetWordAt( File& rFile, Cursor& Cursor ) const;
	std::string GetWordAt( File& rFile, Coordinate Position, Coordinate* pStart, Coordinate* pEnd ) const;
	bool        IsCoordinateInText( File& rFile, Coordinate Position );
	void        AdjustCursorIfInText( File& rFile, Cursor& rCursor, int Line, int XOffset );
	void        AdjustCursor( File& rFile, Cursor& Cursor, int XOffset );
	void        SetSelectionLine( File& rFile, int Line );
	void        SetSelection( File& rFile, Coordinate Start, Coordinate End, int Cursor );
	Coordinate  GetCoordinate( File& rFile, ImVec2 Position, bool RelativeToEditor = false );
	Coordinate  CalculateTabAlignment( File& rFile, Coordinate FromPosition );
	float       CalculateTabAlignmentDistance( File& rFile, Coordinate FromPosition );
	void        AdjustCursors( File& rFile, int Cursor, int XOffset, int YOffset );
	void        YeetDuplicateCursors( File& rFile );
	void        DisableIntersectingSelections( File& rFile, int Cursor );
	void        DeleteDisabledCursor( File& rFile );
	void        DeleteSelection( File& rFile, int Cursor );
	void        Enter( File& rFile );
	void        Backspace( File& rFile );
	void        Del( File& rFile );
	void        Tab( File& rFile, bool Shift );
	void        EnterTextStuff( File& rFile, char C, bool Shift = false );
	void        MoveUp( File& rFile, bool Shift );
	void        MoveDown( File& rFile, bool Shift );
	void        MoveRight( File& rFile, bool Ctrl, bool Shift );
	void        MoveLeft( File& rFile, bool Ctrl, bool Shift );

	Palette m_Palette;

	//////////////////////////////////////////////////////////////////////////

	ImGuiTabBar* m_pTabBar = nullptr;

	Texture2D m_DraggedBitmapTexture = {};

	std::vector< File > m_Files = {};

}; // TextEdit
