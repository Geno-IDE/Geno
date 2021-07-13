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

#include <clang-c/Index.h>
#include <imgui.h>

#if defined( _WIN32 )
#undef ReplaceFile
#endif // _WIN32

class Drop;
struct ImGuiTabBar;

class TextEdit
{
	GENO_DISABLE_COPY_AND_MOVE( TextEdit );

//////////////////////////////////////////////////////////////////////////

public:
	struct Palette
	{
		uint32_t Default;
		// Syntax highlighting
		uint32_t Punctuation;
		uint32_t Keyword;
		uint32_t Identifier;
		uint32_t Literal;
		uint32_t Comment;
		// Interface
		uint32_t LineNumber;
		uint32_t Cursor;
		uint32_t CursorInsert;
		uint32_t Selection;
		uint32_t CurrentLine;
		uint32_t CurrentLineInactive;
		uint32_t CurrentLineEdge;
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

	enum class CursorInputMode
	{
		Normal,
		Insert
	};

	enum class MultiCursorMode
	{
		Normal,
		Box
	};

	enum class BoxModeDirection
	{
		Up,
		Down,
		None
	};

	struct File
	{
		CXTranslationUnit TranslationUnit;

		std::filesystem::path Path;
		std::string           Text;

		std::vector< Line > Lines;

		bool Open    = true;
		bool Changed = false;

		std::vector< Cursor > Cursors;

		float              LongestLineLength;
		std::vector< int > LongestLines;

		BoxModeDirection BoxModeDir      = BoxModeDirection::None;
		CursorInputMode  CursorMode      = CursorInputMode::Normal;
		MultiCursorMode  CursorMultiMode = MultiCursorMode::Normal;
	}; // File

	//////////////////////////////////////////////////////////////////////////

	 TextEdit( void );
	~TextEdit( void );

	//////////////////////////////////////////////////////////////////////////

	void Show( bool* pOpen );
	void AddFile( const std::filesystem::path& rPath );
	void OnDragDrop( const Drop& rDrop, int X, int Y );
	void SaveFile( File& rFile );
	void ReplaceFile( const std::filesystem::path& rOldPath, const std::filesystem::path& rNewPath );

	const std::filesystem::path& GetActiveFilePath() const { return m_ActiveFilePath; }

	//////////////////////////////////////////////////////////////////////////

	static float FontSize;

private:
	void                SplitLines( File& rFile );
	std::vector< Line > SplitLines( const std::string String, int* Count = nullptr );
	void                JoinLines( File& rFile );
	std::string         GetString( const Line& rLine, int Start, int End );

	typedef Coordinate Scroll;

	struct Properties
	{
		float CharAdvanceY;
		float LineNumMaxWidth;
		float SpaceSize;
		float ScrollX;
		float ScrollY;
		bool  Changes;
		int   CursorBlink;
	} Props;

	bool               RenderEditor( File& rFile );
	void               HandleKeyboardInputs( File& rFile );
	void               HandleMouseInputs( File& rFile );
	ImVec2             GetMousePosition();
	void               SetBoxSelection( File& rFile, int LineIndex, float XPosition );
	void               ScrollToCursor( File& rFile );
	void               CheckLineLengths( File& rFile, int FirstLine, int LastLine );
	float              GetMaxCursorDistance( File& rFile );
	void               CalculeteLineNumMaxWidth( File& rFile );
	bool               HasSelection( File& rFile, int cursor ) const;
	Cursor*            IsCoordinateInSelection( File& rFile, Coordinate Coordinate, int Offset = 0 );
	int                IsLineSelected( File& rFile, int line, Coordinate* pStart, Coordinate* pEnd ) const;
	float              GetDistance( File& rFile, Coordinate Position ) const;
	std::string        GetWordAt( File& rFile, Cursor& Cursor ) const;
	std::string        GetWordAt( File& rFile, Coordinate Position, Coordinate* pStart, Coordinate* pEnd ) const;
	bool               IsCoordinateInText( File& rFile, Coordinate Position );
	void               AdjustCursorIfInText( File& rFile, Cursor& rCursor, int Line, int XOffset );
	void               AdjustCursor( Cursor& Cursor, int XOffset );
	void               SetSelectionLine( File& rFile, int Line );
	void               SetSelection( File& rFile, Coordinate Start, Coordinate End, int Cursor );
	int                GetCoordinateY( File& rFile, float YPosition );
	int                GetCoordinateX( File& rFile, int LineIndex, float XPosition, bool AllowPastLine = false );
	Coordinate         GetCoordinate( File& rFile, ImVec2 Position, bool AllowPastLine = false );
	Coordinate         CalculateTabAlignment( File& rFile, Coordinate FromPosition );
	float              CalculateTabAlignmentDistance( File& rFile, Coordinate FromPosition );
	void               AdjustCursors( File& rFile, int Cursor, int XOffset, int YOffset );
	void               YeetDuplicateCursors( File& rFile );
	void               DisableIntersectionsInSelection( File& rFile, int Cursor );
	void               DeleteDisabledCursor( File& rFile );
	void               DeleteSelection( File& rFile, int Cursor );
	void               Enter( File& rFile );
	void               Backspace( File& rFile );
	void               Backspace( File& rFile, int CursorIndex, bool DeleteLine );
	void               Del( File& rFile );
	void               Del( File& rFile, int CursorIndex, bool DeleteLine );
	void               Tab( File& rFile, bool Shift );
	void               Tab( File& rFile, bool Shift, int CursorIndex );
	void               PrepareBoxModeForInput( File& rFile );
	void               EnterTextStuff( File& rFile, char C );
	void               EnterTextStuff( File& rFile, char C, int CursorIndex );
	void               MoveUp( File& rFile, bool Shift, bool Alt );
	void               MoveDown( File& rFile, bool Shift, bool Alt );
	void               MoveRight( File& rFile, bool Ctrl, bool Shift, bool Alt );
	void               MoveLeft( File& rFile, bool Ctrl, bool Shift, bool Alt );
	void               Home( File& rFile, bool Ctrl, bool Shift );
	void               End( File& rFile, bool Ctrl, bool Shift );
	void               Esc( File& rFile );
	void               Copy( File& rFile, bool Cut );
	void               Paste( File& rFile );
	void               SwapLines( File& rFile, bool Up );
	std::vector< int > CursorsInText( File& rFile );
	std::vector< int > CursorsNotInText( File& rFile );
	CXCursor           GetClangCursor( File& rFile, Cursor& rCursor );
	uint32_t           GlyphColorFromTokenKind( CXTokenKind TokenKind );

	Palette m_Palette;

	//////////////////////////////////////////////////////////////////////////

	CXIndex m_ClangIndex = { };

	ImGuiTabBar* m_pTabBar = nullptr;

	Texture2D m_DraggedBitmapTexture = {};

	std::vector< File > m_Files = {};

	std::filesystem::path m_ActiveFilePath = {};

}; // TextEdit
