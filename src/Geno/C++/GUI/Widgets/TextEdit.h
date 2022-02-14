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
#include <thread>
#include <mutex>

#include <imgui.h>
#include <imgui_internal.h>

#if defined( _WIN32 )
#include <Windows.h>
#undef ReplaceFile
#endif // _WIN32

class Drop;
class SearchDialog;
class SearchResultGroups;
struct SearchInstance;
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
		unsigned int CursorInsert;
		unsigned int Selection;
		unsigned int SearchHighlight;
		unsigned int SearchActive;
		unsigned int CurrentLine;
		unsigned int CurrentLineInactive;
		unsigned int CurrentLineEdge;
	};

	struct Glyph
	{
		unsigned int Color;
		char         C;

		Glyph( char C, unsigned int Color )
			: Color( Color )
			, C( C )
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
		bool Main     = false;
		bool Last     = false;
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

	struct LineSelectionItem
	{
		enum
		{
			None,
			Selection,
			Search,
			SearchActive
		};

		Coordinate Start;
		Coordinate End;

		int Type;

		LineSelectionItem( int Type = None )
			: Type( Type )
		{
		}
	};

	enum class CommandType
	{
		None,
		EnterText,
		Backspace,
		Del,
		DeleteLines,
		InsertLines,
		Enter,
		Tab,
		TabAllCursors,
		RestoreCursor,
		Delete,
		SwapLines
	};

	class SubCommand
	{
	public:
		SubCommand( CommandType Type )
			: Type( Type )
			, CursorIndex( -1 )
			, Character( 0, 0 )
			, NoMove( false )
		{
		}

		CommandType Type;

		Cursor CursorCopy;
		int    CursorIndex;

		Coordinate Start = Coordinate( -1, -1 );
		Coordinate End   = Coordinate( -1, -1 );

		std::vector< Line > Lines;
		Glyph               Character;

		union
		{
			bool NoMove;
			bool Shift;
			bool IgnoreSelection;
			bool Up;
		};
	};

	class Command
	{
	public:
		CommandType Type;

		Command( CommandType Type )
			: Type( Type )
		{
		}

		~Command()
		{
			ClearSubCommands();
		}

		void AddSubCommand( SubCommand* pSubCommand )
		{
			SubCommands.push_back( pSubCommand );
		}

		void ClearSubCommands()
		{
			for( SubCommand* pSub : SubCommands )
			{
				delete pSub;
			}

			SubCommands.clear();
		}

		std::vector< Cursor >      Cursors;
		std::vector< SubCommand* > SubCommands;
		BoxModeDirection           BoxModeDir;
		CursorInputMode            CursorMode;
		MultiCursorMode            CursorMultiMode;
	};

	struct File;

	class CommandStack
	{
	public:
		Command* GetCommand( File* pFile, CommandType Type = CommandType::None )
		{
			if( pCurrentCommand ) return pCurrentCommand;

			pCurrentCommand = new Command( Type );

			pCurrentCommand->BoxModeDir      = pFile->BoxModeDir;
			pCurrentCommand->CursorMode      = pFile->CursorMode;
			pCurrentCommand->CursorMultiMode = pFile->CursorMultiMode;

			return pCurrentCommand;
		}

		void FinishCommand( const std::vector< Cursor >& rCursors )
		{
			GENO_ASSERT( pCurrentCommand != nullptr );

			if( pCurrentCommand->SubCommands.size() == 0 )
			{
				delete pCurrentCommand;
				pCurrentCommand = nullptr;
				return;
			}

			int Count = ( int )Commands.size();

			if( CommandInProcess )
			{
				Commands[ Location ] = pCurrentCommand;
			}
			else if( Location == Count - 1 )
			{
				Commands.push_back( pCurrentCommand );
				Location++;
			}
			else if( Location < ( int )Count )
			{
				for( int i = Location + 1; i < ( int )Commands.size(); i++ )
					delete Commands[ i ];

				Commands.erase( Commands.begin() + ( Location + 1 ), Commands.end() );
				Commands.push_back( pCurrentCommand );
				Location++;
			}

			pCurrentCommand->Cursors = rCursors;
			pCurrentCommand          = nullptr;
		}

	public:
		bool                    CommandInProcess = false;
		Command*                pCurrentCommand  = nullptr;
		int                     Location         = -1;
		std::vector< Command* > Commands;
	};

	struct File
	{
		std::filesystem::path Path;
		std::string           Text;

		std::vector< Line > Lines;

		bool Open    = true;
		bool Changed = false;

		std::vector< Cursor > Cursors;

		float              LongestLineLength;
		std::vector< int > LongestLines;

		SearchDialog* SearchDiag;

		CommandStack CmdStack;

		BoxModeDirection BoxModeDir      = BoxModeDirection::None;
		CursorInputMode  CursorMode      = CursorInputMode::Normal;
		MultiCursorMode  CursorMultiMode = MultiCursorMode::Normal;
	}; // File

	//////////////////////////////////////////////////////////////////////////

	TextEdit( void );

	//////////////////////////////////////////////////////////////////////////

	void Show( bool* pOpen );
	void AddFile( const std::filesystem::path& rPath );
	void OnDragDrop( const Drop& rDrop, int X, int Y );
	void SaveFile( File& rFile );
	void ReplaceFile( const std::filesystem::path& rOldPath, const std::filesystem::path& rNewPath );

	const std::filesystem::path& GetActiveFilePath() const { return m_ActiveFilePath; }

	//////////////////////////////////////////////////////////////////////////

	static float FontSize;

	std::vector< File > Files = { };

private:
	void                SplitLines( File& rFile );
	std::vector< Line > SplitLines( const std::string String, int* Count = nullptr );
	void                JoinLines( File& rFile );
	std::string         GetString( const Line& rLine, int Start, int End );
	Line                SubLine( const Line& rLine, int Start, int End );

	typedef Coordinate Scroll;

	struct Properties
	{
		float  CharAdvanceY;
		float  LineNumMaxWidth;
		float  SpaceSize;
		float  ScrollX;
		float  ScrollY;
		ImVec2 WindowSize;
		ImVec2 WindowOrigin;
		bool   Changes;
		int    CursorBlink;
	} Props;

	enum class InsertLineMode
	{
		Default,
		Start,
		SelectionStart,
		SelectionEnd
	};

	bool                             RenderEditor( File& rFile );
	void                             HandleKeyboardInputs( File& rFile );
	void                             HandleMouseInputs( File& rFile );
	ImVec2                           GetMousePosition();
	void                             SetBoxSelection( File& rFile, int LineIndex, float XPosition );
	void                             AddCursor( File& rFile, Cursor& rCursor );
	Cursor&                          GetLastAddedCursor( File& rFile );
	Cursor&                          GetMainCursor( File& rFile );
	void                             EraseAllCursors( File& rFile, bool ExcludeMainCursor = false );
	int                              GetCursorIndex( File& rFile, Cursor& rCursor );
	void                             ScrollToCursor( File& rFile );
	void                             ScrollTo( File& rFile, Coordinate Position );
	void                             ScrollTo( File& rFile, Coordinate Position, ImGuiWindow* pWindow );
	void                             CheckLineLengths( File& rFile, int FirstLine, int LastLine );
	float                            GetMaxCursorDistance( File& rFile );
	void                             CalculeteLineNumMaxWidth( File& rFile );
	bool                             HasSelection( File& rFile, int cursor ) const;
	Cursor*                          IsCoordinateInSelection( File& rFile, Coordinate Coordinate, int Offset = 0 );
	LineSelectionItem                IsSelectionOnLine( File& rFile, int LineIndex, Coordinate Start, Coordinate End ) const;
	std::vector< LineSelectionItem > IsLineSelected( File& rFile, int LineIndex ) const;
	float                            GetDistance( File& rFile, Coordinate Position ) const;
	std::string                      GetWordAt( File& rFile, Cursor& Cursor ) const;
	std::string                      GetWordAt( File& rFile, Coordinate Position, Coordinate* pStart, Coordinate* pEnd ) const;
	bool                             IsCoordinateInText( File& rFile, Coordinate Position );
	void                             AdjustCursorIfInText( File& rFile, Cursor& rCursor, int Line, int XOffset );
	void                             AdjustCursor( Cursor& Cursor, int XOffset );
	void                             SetSelectionLine( File& rFile, int Line );
	void                             SetSelection( File& rFile, Coordinate Start, Coordinate End, int Cursor );
	void                             SelectAll( File& rFile );
	int                              GetCoordinateY( File& rFile, float YPosition );
	int                              GetCoordinateX( File& rFile, int LineIndex, float XPosition, bool AllowPastLine = false );
	Coordinate                       GetCoordinate( File& rFile, ImVec2 Position, bool AllowPastLine = false );
	Coordinate                       CalculateTabAlignment( File& rFile, Coordinate FromPosition );
	float                            CalculateTabAlignmentDistance( File& rFile, Coordinate FromPosition );
	void                             AdjustCursors( File& rFile, int Cursor, int XOffset, int YOffset );
	void                             YeetDuplicateCursors( File& rFile );
	void                             DisableIntersectionsInSelection( File& rFile, Cursor& rCursor );
	void                             DeleteDisabledCursor( File& rFile );
	void                             DeleteSelection( File& rFile, int CursorIndex );
	void                             DeleteLines( File& rFile, int CursorIndex, Coordinate Start, Coordinate End );
	void                             Enter( File& rFile );
	void                             Enter( File& rFile, int CursorIndex );
	void                             Backspace( File& rFile );
	void                             Backspace( File& rFile, int CursorIndex, bool DeleteLine );
	void                             Del( File& rFile );
	void                             Del( File& rFile, int CursorIndex, bool DeleteLine );
	void                             Tab( File& rFile, bool Shift );
	void                             Tab( File& rFile, bool Shift, int CursorIndex, bool IgnoreSelection = false );
	void                             PrepareBoxModeForInput( File& rFile );
	void                             PrepareBoxModeForInput( File& rFile, int CursorIndex );
	void                             EnterTextStuff( File& rFile, char C, bool NoMove = false );
	void                             EnterTextStuff( File& rFile, char C, int CursorIndex, bool NoMove = false );
	void                             MoveUp( File& rFile, bool Shift, bool Alt );
	void                             MoveDown( File& rFile, bool Shift, bool Alt );
	void                             MoveRight( File& rFile, bool Ctrl, bool Shift, bool Alt );
	void                             MoveLeft( File& rFile, bool Ctrl, bool Shift, bool Alt );
	void                             Home( File& rFile, bool Ctrl, bool Shift );
	void                             End( File& rFile, bool Ctrl, bool Shift );
	void                             Esc( File& rFile );
	void                             Copy( File& rFile, bool Cut );
	void                             Paste( File& rFile );
	void                             InsertLines( File& rFile, int CursorIndex, std::vector< Line >& rLines, InsertLineMode Mode = InsertLineMode::Default );
	void                             InsertLines( File& rFile, Cursor& rCursor, std::vector< Line >& rLines, InsertLineMode Mode = InsertLineMode::Default );
	void                             SwapLines( File& rFile, bool Up );
	std::vector< int >               CursorsInText( File& rFile );
	std::vector< int >               CursorsNotInText( File& rFile );
	bool                             CursorInText( File& rFile, int CursorIndex );
	void                             ClearSearch( File& rFile );
	Coordinate                       SearchInLine( File& rFile, bool CaseSensitive, const std::string& rSearchString, Coordinate LineStart, int SearchStringOffset, std::vector< Glyph* >& rMatches );
	void                             SearchWorker( File* pFile, bool CaseSensitive, bool WholeWord, const std::string* pSearchString, int StartLine, int EndLine, std::vector< LineSelectionItem >* pResult, int* pState );
	void                             SearchManager( File* pFile, bool CaseSensitive, bool WholeWord, SearchInstance* Instance );
	void                             Search( File& rFile, bool CaseSensitve, bool WholeWord, const std::string& rSearchString );
	void                             JoinThreads( File& rFile, bool WaitForUnfinished );
	void                             ShowSearchDialog( File& rFile, ImGuiID FocusId, ImGuiWindow* pWindow );
	void                             Undo( File& rFile );
	void                             Redo( File& rFile );
	void                             ExecuteCommand( File& rFile, Command* pCommand );

	Palette m_Palette;

	//////////////////////////////////////////////////////////////////////////

	ImGuiTabBar* m_pTabBar = nullptr;

	Texture2D m_DraggedBitmapTexture = {};

	std::filesystem::path m_ActiveFilePath = {};

}; // TextEdit

//////////////////////////////////////////////////////////////////////////

class SearchResultGroups
{
public:
	// Results will be grouped in it's own vector for every 1000 lines
	std::vector< std::vector< TextEdit::LineSelectionItem* > > Groups;
	// A vector of all the results in order
	std::vector< TextEdit::LineSelectionItem > Result;

	SearchResultGroups();

	std::vector< TextEdit::LineSelectionItem* >& GetGroup( int LineIndex );
	bool                                         GroupExist( int LineIndex );
	void                                         AddResult( const TextEdit::LineSelectionItem& Item );
	void                                         UpdateGroups();
	void                                         Clear();
	size_t                                       Size();
	bool                                         Empty();

	TextEdit::LineSelectionItem& operator[]( int Index );
}; // SearchResultGroups

//////////////////////////////////////////////////////////////////////////

struct SearchInstance
{
	enum
	{
		Running,
		Stopping,
		Stopped,
		HasResult
	};

	std::thread         Thread;
	std::string         SearchTerm;
	SearchResultGroups* Result;
	int                 State;
}; // SearchInstance

//////////////////////////////////////////////////////////////////////////

class SearchDialog
{
public:
	bool Searching     = false;
	bool CaseSensitive = false;
	bool WholeWord     = false;
	int  ActiveItem    = -1;

	std::string SearchTerm;

	std::vector< SearchInstance* > SearchInstances;
	SearchResultGroups*            SearchResult = nullptr;
}; // SearchDialog
