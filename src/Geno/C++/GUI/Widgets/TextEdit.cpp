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

#include "TextEdit.h"

#include "Application.h"
#include "Common/Drop.h"
#include "Common/LocalAppData.h"
#include "GUI/MainWindow.h"
#include "GUI/Widgets/TitleBar.h"
#include "Discord/DiscordRPC.h"
#include "GUI/Widgets/StatusBar.h"

#include <fstream>
#include <iostream>

#include <misc/cpp/imgui_stdlib.h>

const char* WINDOW_NAME = "Text Edit";

constexpr float TabSize                 = 4.0f;
constexpr float EmptyLineSelectionWidth = 4.0f;
constexpr int   CursorBlink             = 400;
constexpr float DummyExtraX             = 10.0f;
constexpr float DummyExtraY             = 10.0f;
constexpr int   SearchResultGroupSize   = 1000;

float TextEdit::FontSize = 15.0f;

//////////////////////////////////////////////////////////////////////////

TextEdit::TextEdit( void )
{
	// Create tab bar
	{
		ImGuiContext* pContext = ImGui::GetCurrentContext();
		ImGuiID       ID       = ImHashStr( "TextEditTabBar" ); // ImGui::GetID( "TextEditTabBar" );
		m_pTabBar              = pContext->TabBars.GetOrAddByKey( ID );
		m_pTabBar->ID          = ID;
	}

	m_Palette.Default             = 0xFFf4f4f4;
	m_Palette.Keyword             = 0xFF0000F0;
	m_Palette.Number              = 0xFF303030;
	m_Palette.String              = 0xFF9E5817;
	m_Palette.Comment             = 0xFF0f5904;
	m_Palette.LineNumber          = 0xFFF0F0F0;
	m_Palette.Cursor              = 0xFFf8f8f8;
	m_Palette.CursorInsert        = 0x80f8f8f8;
	m_Palette.Selection           = 0x80a06020;
	m_Palette.SearchHighlight     = 0x80002c4f;
	m_Palette.SearchActive        = 0x80f7da5d;
	m_Palette.CurrentLine         = 0x40000000;
	m_Palette.CurrentLineInactive = 0x40808080;
	m_Palette.CurrentLineEdge     = 0x40a0a0a0;

} // TextEdit

//////////////////////////////////////////////////////////////////////////

void TextEdit::Show( bool* pOpen )
{
	ImGuiStyle& rStyle          = ImGui::GetStyle();
	ImVec4      BackgroundColor = rStyle.Colors[ ImGuiCol_WindowBg ];

	// Use a brighter background color if the widget is being drag-hovered
	if( const Drop* pDrop = MainWindow::Instance().GetDraggedDrop() )
	{
		const float DragX = static_cast< float >( MainWindow::Instance().GetDragPosX() );
		const float DragY = static_cast< float >( MainWindow::Instance().GetDragPosY() );

		if( ImGuiWindow* pWindow = ImGui::FindWindowByName( WINDOW_NAME ) )
		{
			if( pWindow->Rect().Contains( ImVec2( DragX, DragY ) ) )
			{
				BackgroundColor = BackgroundColor + ImVec4( 0.1f, 0.1f, 0.1f, 0.1f );

				switch( pDrop->GetType() )
				{
					case Drop::TypeIndex::Bitmap:
					{
						const Drop::Bitmap& rBitmap = pDrop->GetBitmap();

						m_DraggedBitmapTexture.SetPixels( GL_RGBA8, rBitmap.width, rBitmap.height, GL_BGRA, rBitmap.data.get() );

						// Adjust image size and ensure that no dimension is bigger than 200px
						constexpr float ImageMaxSize = 200.0f;
						ImVec2          ImageSize;
						if( rBitmap.width > rBitmap.height ) ImageSize = ImVec2( ImageMaxSize, ImageMaxSize * ( rBitmap.height / static_cast< float >( rBitmap.width ) ) );
						else
							ImageSize = ImVec2( ImageMaxSize * ( rBitmap.width / static_cast< float >( rBitmap.height ) ), ImageMaxSize );

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Image( m_DraggedBitmapTexture.GetID(), ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
						ImGui::EndTooltip();
					}
					break;

					case Drop::TypeIndex::Text:
					{
						const Drop::Text& rText = pDrop->GetText();

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Text( "%ls", rText.c_str() );
						ImGui::EndTooltip();
					}
					break;

					case Drop::TypeIndex::Paths:
					{
						const Drop::Paths& rPaths = pDrop->GetPaths();

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();

						for( const std::filesystem::path& rPath : rPaths )
							ImGui::BulletText( "%s", rPath.string().c_str() );

						ImGui::EndTooltip();
					}
					break;
				}
			}
		}
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, BackgroundColor );
	ImGui::SetNextWindowSize( ImVec2( 350, 196 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( WINDOW_NAME, pOpen, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar ) )
	{
		const int           TabBarFlags  = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_IsFocused;
		const ImGuiContext* pContext     = ImGui::GetCurrentContext();
		const ImGuiWindow*  pWindow      = ImGui::GetCurrentWindow();
		const ImRect        TabBarBounds = ImRect( pWindow->DC.CursorPos.x, pWindow->DC.CursorPos.y, pWindow->WorkRect.Max.x, pWindow->DC.CursorPos.y + pContext->FontSize + pContext->Style.FramePadding.y * 2 );

		if( !Files.empty() && ImGui::BeginTabBarEx( m_pTabBar, TabBarBounds, TabBarFlags, nullptr ) )
		{
			for( File& rFile : Files )
			{
				std::string FileString = rFile.Path.filename().string();

				if( ImGui::BeginTabItem( FileString.c_str(), &rFile.Open, rFile.Changed ? ImGuiTabItemFlags_UnsavedDocument : 0 ) )
				{
					m_ActiveFilePath = rFile.Path;

					DiscordRPC::Instance().m_CurrentFile    = FileString;
					DiscordRPC::Instance().m_CurrentFileExt = rFile.Path.extension().string();

					ImGui::PushFont( MainWindow::Instance().GetFontMono() );

					if( RenderEditor( rFile ) )
					{
						rFile.Changed = true;
					}

					ImGui::PopFont();
					ImGui::EndTabItem();
				}

				if( m_ActiveFilePath == rFile.Path && !rFile.Open )
					m_ActiveFilePath.clear();
			}

			// Clear closed files from list
			for( auto It = Files.begin(); It != Files.end(); )
			{
				if( It->Open ) It++;
				else
					It = Files.erase( It );
			}

			ImGui::EndTabBar();
		}
	}

	if( Files.size() )
	{
		for( auto& rFile : Files )
		{
			if( rFile.Path.string() == m_ActiveFilePath )
			{
				if( rFile.Cursors.size() )
				{
					auto& rCursor = GetMainCursor( rFile );
					int   Length  = static_cast< int >( rFile.Text.size() );
					int   Lines   = static_cast< int >( rFile.Lines.size() );

					StatusBar::Instance().SetCurrentFileInfo( rCursor.Position.x + 1, rCursor.Position.y + 1, Length, Lines );
				}
			}
		}
	}
	else
		StatusBar::Instance().ClearCurrentFileInfo();

	ImGui::End();
	ImGui::PopStyleColor();

} // Show

//////////////////////////////////////////////////////////////////////////

void TextEdit::AddFile( const std::filesystem::path& rPath )
{
	if( !std::filesystem::exists( rPath ) )
	{
		std::cerr << "Failed to add '" << rPath << "' to text-edit. File does not exist.\n";
		return;
	}

	//////////////////////////////////////////////////////////////////////////

	std::ifstream     InputFileStream( rPath, std::ios::binary );
	const std::string Text( ( std::istreambuf_iterator< char >( InputFileStream ) ), std::istreambuf_iterator< char >() );

	for( int i = 0; i < ( int )Files.size(); ++i )
	{
		File& rFile = Files[ i ];

		// Do not need to add rFile to vector if it already exists
		if( rFile.Path == rPath )
		{
			// Select the tab that corresponds to the open rFile
			m_pTabBar->NextSelectedTabId = m_pTabBar->Tabs[ static_cast< int >( i ) ].ID;

			// Update text in case rFile changed externally
			rFile.Text = Text;

			SplitLines( rFile );

			return;
		}
	}

	File File;
	File.Path       = rPath;
	File.Text       = Text;
	File.SearchDiag = new SearchDialog;

	SplitLines( File );

	Files.emplace_back( std::move( File ) );

} // AddFile

//////////////////////////////////////////////////////////////////////////

void TextEdit::OnDragDrop( const Drop& rDrop, int X, int Y )
{
	ImGuiWindow* pWindow = ImGui::FindWindowByName( WINDOW_NAME );

	if( pWindow && pWindow->Rect().Contains( ImVec2( static_cast< float >( X ), static_cast< float >( Y ) ) ) )
	{
		switch( rDrop.GetType() )
		{
			case Drop::TypeIndex::Paths:
			{
				const Drop::Paths& rPaths = rDrop.GetPaths();

				for( const std::filesystem::path& rPath : rPaths )
					AddFile( rPath );
			}
			break;

			default:
			{
			}
			break;
		}
	}

} // OnDragDrop

//////////////////////////////////////////////////////////////////////////

void TextEdit::ReplaceFile( const std::filesystem::path& rOldPath, const std::filesystem::path& rNewPath )
{
	// Replace Renamed File
	for( auto& rFile : Files )
	{
		if( rFile.Path == rOldPath )
		{
			rFile.Path = rNewPath;
			break;
		}
	}

} //ReplaceFile

//////////////////////////////////////////////////////////////////////////

void TextEdit::SaveFile( File& rFile )
{
	if( !rFile.Changed ) return;

	JoinLines( rFile );

	std::ofstream ofs( rFile.Path, std::ios::binary | std::ios::trunc );
	ofs << rFile.Text;

	rFile.Changed = false;

	StatusBar::Instance().SetText( "Item saved : " + rFile.Path.string() );

} // SaveFile

//////////////////////////////////////////////////////////////////////////

void TextEdit::SplitLines( File& rFile )
{
	rFile.Lines.clear();

	rFile.Lines = SplitLines( rFile.Text );

} // SplitLines

//////////////////////////////////////////////////////////////////////////

std::vector< TextEdit::Line > TextEdit::SplitLines( const std::string String, int* Count )
{
	Line                LineBuffer;
	std::vector< Line > Lines;

	for( int i = 0; i < ( int )String.length(); i++ )
	{
		const char c = String[ i ];

		if( c == '\n' || c == '\r' )
		{
			if( c == '\r' ) i++;

			Lines.emplace_back( std::move( LineBuffer ) );
		}
		else
		{
			LineBuffer.push_back( Glyph( c, m_Palette.Default ) );
		}
	}

	if( !LineBuffer.empty() ) Lines.push_back( LineBuffer );

	if( Count )
	{
		*Count = ( int )Lines.size();
	}

	// We make a lot of assumptions that line vectors contain at least one element.
	// Let's make sure the vector is never empty, even for empty input strings.
	if( Lines.empty() ) Lines.emplace_back();

	return Lines;
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::JoinLines( File& rFile )
{
	rFile.Text.clear();

	for( size_t i = 0; i < rFile.Lines.size(); i++ )
	{
		Line&       rLine = rFile.Lines[ i ];
		std::string Line  = GetString( rLine, 0, ( int )rLine.size() );

		rFile.Text.append( Line );
		rFile.Text.push_back( '\n' );
	}

} // JoinLines

//////////////////////////////////////////////////////////////////////////

std::string TextEdit::GetString( const Line& rLine, int Start, int End )
{
	std::string Result;
	int         LineSize = ( int )rLine.size();

	End = End > LineSize ? LineSize : End;

	for( int i = Start; i < End; i++ )
	{
		Result.push_back( rLine[ i ].C );
	}

	return Result;

} // GetLineString

//////////////////////////////////////////////////////////////////////////

TextEdit::Line TextEdit::SubLine( const Line& rLine, int Start, int End )
{
	Line Tmp;

	for( int i = Start; i < End; i++ )
	{
		Tmp.push_back( rLine[ i ] );
	}

	return Tmp;
} // SubLine

//////////////////////////////////////////////////////////////////////////

bool TextEdit::RenderEditor( File& rFile )
{
	Props.Changes = false;
	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4( 0xFF101010 ) );
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

	ImGui::PushAllowKeyboardFocus( true );

	CalculeteLineNumMaxWidth( rFile );

	Props.CharAdvanceY = ImGui::GetTextLineHeightWithSpacing();
	Props.SpaceSize    = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, " " ).x;

	ImVec2 Size         = ImGui::GetContentRegionMax();
	ImVec2 ScreenCursor = ImGui::GetCursorScreenPos();

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	ImGui::SetCursorScreenPos( ImVec2( ScreenCursor.x + Props.LineNumMaxWidth, ScreenCursor.y ) );
	ImGui::BeginChild( "##TextEditor", ImVec2( Size.x - Props.LineNumMaxWidth, 0 ), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar );

	ImGuiID      FocusId = ImGui::GetFocusID();
	ImGuiWindow* pWindow = ImGui::GetCurrentWindow();

	Props.WindowSize   = ImGui::GetContentRegionAvail();
	Props.WindowOrigin = ImGui::GetCursorScreenPos();
	Props.ScrollX      = ImGui::GetScrollX();
	Props.ScrollY      = ImGui::GetScrollY();

	HandleKeyboardInputs( rFile );
	HandleMouseInputs( rFile );

	int FirstLine = ( int )( Props.ScrollY / Props.CharAdvanceY );
	int LastLine  = std::min( FirstLine + ( int )( Size.y / Props.CharAdvanceY + 2 ), ( int )rFile.Lines.size() - 1 );

	for( int i = FirstLine; i <= LastLine; i++ )
	{
		ImVec2 Pos( ScreenCursor.x + Props.LineNumMaxWidth - Props.ScrollX, ScreenCursor.y + ( i - FirstLine ) * Props.CharAdvanceY );
		Line&  rLine = rFile.Lines[ i ];

		std::vector< LineSelectionItem > Selections = IsLineSelected( rFile, i );

		for( LineSelectionItem& rItem : Selections )
		{
			float StartX = 0.0f;
			float EndX   = EmptyLineSelectionWidth;

			if( rItem.Start != rItem.End )
			{
				StartX = GetDistance( rFile, rItem.Start );
				EndX   = GetDistance( rFile, rItem.End );
			}

			ImVec2 Start( ScreenCursor.x + Props.LineNumMaxWidth + StartX - Props.ScrollX, Pos.y );
			ImVec2 End( ScreenCursor.x + Props.LineNumMaxWidth + EndX - Props.ScrollX, Pos.y + Props.CharAdvanceY );

			pDrawList->AddRectFilled( Start, End, rItem.Type == LineSelectionItem::Search ? m_Palette.SearchHighlight : m_Palette.Selection );
		}

		for( int j = 0; j < ( int )rFile.Cursors.size(); j++ )
		{
			Cursor& rCursor = rFile.Cursors[ j ];

			if( rCursor.Disabled ) continue;

			if( rCursor.Position.y == i )
			{
				bool Focus = ImGui::IsWindowFocused();

				if( !HasSelection( rFile, j ) && ( rFile.CursorMultiMode == MultiCursorMode::Normal || rFile.Cursors.size() == 1 ) )
				{
					ImVec2 Start( ScreenCursor.x + Props.LineNumMaxWidth - 2, Pos.y );
					ImVec2 End( ScreenCursor.x + Size.x - ImGui::GetStyle().ScrollbarSize, Pos.y + Props.CharAdvanceY );

					pDrawList->AddRectFilled( Start, End, Focus ? m_Palette.CurrentLine : m_Palette.CurrentLineInactive );
					pDrawList->AddRect( Start, End, m_Palette.CurrentLineEdge );
				}

				if( Focus )
				{
					static auto Start   = std::chrono::system_clock::now();
					auto        Now     = std::chrono::system_clock::now();
					long long   Elapsed = std::chrono::duration_cast< std::chrono::milliseconds >( Now - Start ).count();

					if( Elapsed >= Props.CursorBlink )
					{
						Elapsed -= Props.CursorBlink;

						float  CursorPos = GetDistance( rFile, rCursor.Position );
						ImVec2 cStart( Pos.x + CursorPos, Pos.y );
						ImVec2 cEnd( cStart.x, cStart.y + Props.CharAdvanceY - 1 );

						unsigned int Color = 0;

						switch( rFile.CursorMode )
						{
							case CursorInputMode::Normal:
								cEnd.x += 1.0f;
								Color = m_Palette.Cursor;
								break;
							case CursorInputMode::Insert:
								cEnd.x += Props.SpaceSize - 0.75f;
								Color = m_Palette.CursorInsert;
								break;
						}

						pDrawList->AddRectFilled( cStart, cEnd, Color );

						if( Props.CursorBlink ? Elapsed >= Props.CursorBlink : Elapsed >= CursorBlink * 2 )
						{
							Start             = Now;
							Props.CursorBlink = CursorBlink;
						}
					}
				}
			}
		}

		std::string StringBuffer;

		float        XOffset   = 0.0f;
		unsigned int PrevColor = ( unsigned int )rLine.size() ? rLine[ 0 ].Color : m_Palette.Default;

		for( Glyph& rGlyph : rLine )
		{
			if( rGlyph.Color != PrevColor || ( rGlyph.C == '\t' && !StringBuffer.empty() ) )
			{
				ImVec2 RenderStart( Pos.x + XOffset, Pos.y );

				pDrawList->AddText( RenderStart, PrevColor, StringBuffer.c_str() );
				float TextWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, StringBuffer.c_str() ).x;
				XOffset += TextWidth;
				StringBuffer.clear();

				PrevColor = rGlyph.Color;

				if( rGlyph.C == '\t' )
				{
					float Tab = TabSize * Props.SpaceSize;

					XOffset += Tab;

					float Fraction = XOffset / Tab;
					Fraction       = Fraction - floorf( Fraction );

					XOffset -= Tab * Fraction;
				}
				else
				{
					StringBuffer.push_back( rGlyph.C );
				}
			}
			else if( rGlyph.C == '\t' )
			{
				XOffset += TabSize * Props.SpaceSize;
			}
			else
			{
				StringBuffer.push_back( rGlyph.C );
			}
		}

		if( !StringBuffer.empty() )
		{
			ImVec2 RenderStart( Pos.x + XOffset, Pos.y );

			pDrawList->AddText( RenderStart, PrevColor, StringBuffer.c_str() );
			float TextWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, StringBuffer.c_str() ).x;
			XOffset += TextWidth;
			StringBuffer.clear();
		}
	}

	CheckLineLengths( rFile, FirstLine, LastLine );

	float Width = GetMaxCursorDistance( rFile );

	if( rFile.LongestLineLength > Width ) Width = rFile.LongestLineLength;

	ImGui::Dummy( ImVec2( Width + DummyExtraX, ( rFile.Lines.size() + DummyExtraY ) * Props.CharAdvanceY ) );

	ImGui::PopAllowKeyboardFocus();
	ImGui::EndChild();

	//Render Line numbers
	ImGui::SetCursorScreenPos( ImVec2( ScreenCursor.x - 2, ScreenCursor.y ) );
	ImGui::BeginChild( "##LineNumbers", ImVec2( Props.LineNumMaxWidth, Size.y + 2 ), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

	char Buf[ 32 ];

	for( int i = FirstLine; i <= LastLine; i++ )
	{
		sprintf( Buf, "%u | ", i + 1 );

		const float CurrentLineNumWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, Buf ).x;
		ImVec2      Pos( ScreenCursor.x + Props.LineNumMaxWidth - CurrentLineNumWidth, ScreenCursor.y + ( i - FirstLine ) * Props.CharAdvanceY );

		pDrawList->AddText( Pos, m_Palette.LineNumber, Buf );
	}

	ImGui::EndChild();

	if( rFile.SearchDiag->Searching )
	{
		ImGui::SetCursorScreenPos( ScreenCursor );
		ShowSearchDialog( rFile, FocusId, pWindow );
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	return Props.Changes;

} // RenderEditor

//////////////////////////////////////////////////////////////////////////

void TextEdit::HandleKeyboardInputs( File& rFile )
{
	if( ImGui::IsWindowFocused() )
	{
		ImGuiIO& rIO = ImGui::GetIO();

		bool Shift = rIO.KeyShift;
		bool Ctrl  = rIO.KeyCtrl;
		bool Alt   = rIO.KeyAlt;

		// Keyboard Inputs
		rIO.WantCaptureKeyboard = true;
		rIO.WantTextInput       = true;

		if( !Shift && !Ctrl && !Alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Enter ) ) )
			Enter( rFile );
		else if( !Shift && !Ctrl && !Alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Backspace ) ) )
			Backspace( rFile );
		else if( !Ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow ) ) )
			MoveUp( rFile, Shift, Alt );
		else if( !Ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_DownArrow ) ) )
			MoveDown( rFile, Shift, Alt );
		else if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow ) ) )
			MoveRight( rFile, Ctrl, Shift, Alt );
		else if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow ) ) )
			MoveLeft( rFile, Ctrl, Shift, Alt );
		else if( !Alt && !Ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Delete ) ) )
			Del( rFile );
		else if( !Alt && !Ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Tab ) ) )
			Tab( rFile, Shift );
		else if( !Alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Home ) ) )
			Home( rFile, Ctrl, Shift );
		else if( !Alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_End ) ) )
			End( rFile, Ctrl, Shift );
		else if( !Alt && !Ctrl && !Shift && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Escape ) ) )
			Esc( rFile );
		else if( !Alt && !Shift && Ctrl && ImGui::IsKeyPressed( 'S' ) )
			SaveFile( rFile );
		else if( !Alt && !Shift && Ctrl && ImGui::IsKeyPressed( 'C' ) )
			Copy( rFile, false );
		else if( !Alt && !Shift && Ctrl && ImGui::IsKeyPressed( 'X' ) )
			Copy( rFile, true );
		else if( !Alt && !Shift && Ctrl && ImGui::IsKeyPressed( 'V' ) )
			Paste( rFile );
		else if( !Alt && !Ctrl && !Shift && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Insert ) ) && rFile.CursorMultiMode == MultiCursorMode::Normal )
			rFile.CursorMode = rFile.CursorMode == CursorInputMode::Normal ? CursorInputMode::Insert : CursorInputMode::Normal;
		else if( !Alt && Ctrl && !Shift && ImGui::IsKeyPressed( 'A' ) )
			SelectAll( rFile );
		else if( !Alt && Ctrl && !Shift && ImGui::IsKeyPressed( 'F' ) )
			rFile.SearchDiag->Searching = true;
		else if( !Alt && Ctrl && !Shift && ImGui::IsKeyPressed( 'Z' ) )
			Undo( rFile );
		else if( !Alt && Ctrl && !Shift && ImGui::IsKeyPressed( 'Y' ) )
			Redo( rFile );

		for( int i = 0; i < rIO.InputQueueCharacters.Size; i++ )
		{
			char c = ( char )rIO.InputQueueCharacters[ i ];

			EnterTextStuff( rFile, c );
		}
	}

} // HandleKeyboardInputs

//////////////////////////////////////////////////////////////////////////

void TextEdit::HandleMouseInputs( File& rFile )
{
	ImGuiIO& rIO = ImGui::GetIO();

	bool Shift = rIO.KeyShift;
	bool Ctrl  = rIO.KeyCtrl;
	bool Alt   = rIO.KeyAlt;

	if( ImGui::IsWindowHovered() )
	{
		// Mouse Inputs
		ImGui::SetMouseCursor( ImGuiMouseCursor_TextInput );

		bool   Clicked       = ImGui::IsMouseClicked( ImGuiMouseButton_Left );
		bool   DoubleClicked = ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left );
		bool   Dragged       = ImGui::IsMouseDragging( ImGuiMouseButton_Left );
		ImVec2 MouseCoords   = GetMousePosition();

		if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) || ( Props.Changes && ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) )
		{
			if( Props.Changes )
				GetLastAddedCursor( rFile ).SelectionOrigin = GetCoordinate( rFile, MouseCoords );

			DeleteDisabledCursor( rFile );
		}

		if( DoubleClicked )
		{
			Cursor& rLastCursor = GetLastAddedCursor( rFile );

			std::string Word = GetWordAt( rFile, rLastCursor );

			if( Word.empty() ) return;

			rLastCursor.Position = rLastCursor.SelectionEnd;
		}
		else if( Clicked )
		{
			Coordinate NewPosition = GetCoordinate( rFile, MouseCoords, Alt && !Ctrl );

			if( Ctrl && !( Alt || Shift ) )
			{
			}
			else if( Shift && !Ctrl && rFile.CursorMultiMode == MultiCursorMode::Normal )
			{
				if( Alt )
				{
					rFile.CursorMultiMode = MultiCursorMode::Box;
					HandleMouseInputs( rFile );
					return;
				}

				EraseAllCursors( rFile, true );

				Cursor& rCursor = rFile.Cursors[ 0 ];

				if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) )
				{
					if( NewPosition > rCursor.Position )
					{
						rCursor.SelectionOrigin = rCursor.SelectionStart = rCursor.Position;
						rCursor.SelectionEnd = rCursor.Position = NewPosition;
					}
					else
					{
						rCursor.SelectionOrigin = rCursor.SelectionEnd = rCursor.Position;
						rCursor.SelectionStart = rCursor.Position = NewPosition;
					}
				}
				else
				{
					if( NewPosition > rCursor.SelectionOrigin )
					{
						rCursor.SelectionStart = rCursor.SelectionOrigin;
						rCursor.SelectionEnd = rCursor.Position = NewPosition;
					}
					else
					{
						rCursor.SelectionEnd   = rCursor.SelectionOrigin;
						rCursor.SelectionStart = rCursor.Position = NewPosition;
					}
				}
			}
			else if( Shift && !Ctrl && rFile.CursorMultiMode == MultiCursorMode::Box )
			{
				Cursor& rCursor = GetMainCursor( rFile );

				if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) )
				{
					rCursor.SelectionOrigin = rCursor.Position;
				}

				Props.CursorBlink = 0;

				SetBoxSelection( rFile, NewPosition.y, MouseCoords.x );
			}
			else
			{
				if( !( Ctrl && Alt ) ) rFile.Cursors.clear();

				Cursor NewCursor;

				NewCursor.Position       = NewPosition;
				NewCursor.SelectionStart = Coordinate( 0, 0 );
				NewCursor.SelectionEnd   = Coordinate( 0, 0 );
				NewCursor.Main           = rFile.Cursors.size() == 0;

				AddCursor( rFile, NewCursor );

				Props.CursorBlink     = 0;
				rFile.CursorMultiMode = MultiCursorMode::Normal;

				if( Alt && !Ctrl )
				{
					rFile.CursorMultiMode = MultiCursorMode::Box;
					rFile.CursorMode      = CursorInputMode::Normal;
				}

				YeetDuplicateCursors( rFile );
			}
		}
		else if( Dragged )
		{
			if( rFile.CursorMultiMode == MultiCursorMode::Box )
			{
				int     Line    = GetCoordinateY( rFile, MouseCoords.y );
				Cursor& rCursor = GetMainCursor( rFile );

				if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) )
				{
					rCursor.SelectionOrigin = rCursor.Position;
				}

				Coordinate TmpPos = Coordinate( GetCoordinateX( rFile, rCursor.Position.y, MouseCoords.x, true ), rCursor.Position.y );

				if( rCursor.Position != TmpPos ) Props.CursorBlink = 0;

				SetBoxSelection( rFile, Line, MouseCoords.x );
			}
			else
			{
				Coordinate Pos     = GetCoordinate( rFile, MouseCoords );
				Cursor&    rCursor = GetLastAddedCursor( rFile );

				if( rCursor.SelectionOrigin != Coordinate( ~0, ~0 ) )
				{
					if( Pos > rCursor.SelectionOrigin )
					{
						rCursor.SelectionEnd   = Pos;
						rCursor.SelectionStart = rCursor.SelectionOrigin;
					}
					else
					{
						rCursor.SelectionStart = Pos;
						rCursor.SelectionEnd   = rCursor.SelectionOrigin;
					}

					if( rCursor.Position != Pos ) Props.CursorBlink = 0;

					rCursor.Position = Pos;

					DisableIntersectionsInSelection( rFile, rCursor );
				}
				else
				{
					rCursor.SelectionOrigin = rCursor.Position;
				}
			}
		}

		if( rIO.MouseWheel != 0 && rIO.KeyCtrl && !rIO.KeyAlt && !rIO.KeyShift )
			FontSize += rIO.MouseWheel;
	}

} // HandleMouseInputs

//////////////////////////////////////////////////////////////////////////

ImVec2 TextEdit::GetMousePosition()
{
	ImVec2 Origin   = Props.WindowOrigin;
	ImVec2 MousePos = ImGui::GetMousePos();

	MousePos.x -= Origin.x;
	MousePos.y -= Origin.y;

	return MousePos;
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::SetBoxSelection( File& rFile, int LineIndex, float XPosition )
{
	EraseAllCursors( rFile, true );

	Cursor CurrCursor      = GetMainCursor( rFile );
	float  OriginXDistance = GetDistance( rFile, CurrCursor.SelectionOrigin );

	if( LineIndex > CurrCursor.SelectionOrigin.y )
	{
		rFile.BoxModeDir = BoxModeDirection::Down;

		for( int i = CurrCursor.SelectionOrigin.y + 1; i <= LineIndex; i++ )
		{
			Cursor Cursor;

			Cursor.Position        = Coordinate( GetCoordinateX( rFile, i, XPosition, true ), i );
			Cursor.SelectionOrigin = Coordinate( GetCoordinateX( rFile, i, OriginXDistance, true ), i );

			int Dir = Cursor.Position.x - Cursor.SelectionOrigin.x;

			if( Dir > 0 )
			{
				Cursor.SelectionStart = Cursor.SelectionOrigin;
				Cursor.SelectionEnd   = Cursor.Position;
			}
			else if( Dir < 0 )
			{
				Cursor.SelectionStart = Cursor.Position;
				Cursor.SelectionEnd   = Cursor.SelectionOrigin;
			}
			else
			{
				Cursor.SelectionStart = Coordinate( 0, 0 );
				Cursor.SelectionEnd   = Coordinate( 0, 0 );
			}

			AddCursor( rFile, Cursor );
		}
	}
	else if( LineIndex < CurrCursor.SelectionOrigin.y )
	{
		rFile.BoxModeDir = BoxModeDirection::Up;

		for( int i = CurrCursor.SelectionOrigin.y + -1; i >= LineIndex; i-- )
		{
			Cursor Cursor;

			Cursor.Position        = Coordinate( GetCoordinateX( rFile, i, XPosition, true ), i );
			Cursor.SelectionOrigin = Coordinate( GetCoordinateX( rFile, i, OriginXDistance, true ), i );

			int Dir = Cursor.Position.x - Cursor.SelectionOrigin.x;

			if( Dir > 0 )
			{
				Cursor.SelectionStart = Cursor.SelectionOrigin;
				Cursor.SelectionEnd   = Cursor.Position;
			}
			else if( Dir < 0 )
			{
				Cursor.SelectionStart = Cursor.Position;
				Cursor.SelectionEnd   = Cursor.SelectionOrigin;
			}
			else
			{
				Cursor.SelectionStart = Coordinate( 0, 0 );
				Cursor.SelectionEnd   = Coordinate( 0, 0 );
			}

			AddCursor( rFile, Cursor );
		}
	}
	else
	{
		rFile.BoxModeDir = BoxModeDirection::None;
	}

	Cursor& rCursor = GetMainCursor( rFile );

	int PosX = GetCoordinateX( rFile, rCursor.Position.y, XPosition, true );

	rCursor.Position.x = PosX;

	if( PosX > rCursor.SelectionOrigin.x )
	{
		rCursor.SelectionStart = rCursor.SelectionOrigin;
		rCursor.SelectionEnd   = Coordinate( PosX, rCursor.Position.y );
	}
	else if( PosX < rCursor.SelectionOrigin.x )
	{
		rCursor.SelectionEnd   = rCursor.SelectionOrigin;
		rCursor.SelectionStart = Coordinate( PosX, rCursor.Position.y );
		;
	}
	else
	{
		rCursor.SelectionStart = Coordinate( 0, 0 );
		rCursor.SelectionEnd   = Coordinate( 0, 0 );
	}

} // SetBoxSelection

//////////////////////////////////////////////////////////////////////////

void TextEdit::AddCursor( File& rFile, Cursor& rCursor )
{
	rCursor.Last = true;

	for( int i = ( int )rFile.Cursors.size() - 1; i >= 0; i-- )
	{
		Cursor& rCur = rFile.Cursors[ i ];

		if( rCursor.Position > rCur.Position )
		{
			rFile.Cursors.insert( rFile.Cursors.begin() + i + 1, rCursor );

			for( int j = 0; j <= i; j++ )
			{
				rFile.Cursors[ j ].Last = false;
			}

			return;
		}

		rCur.Last = false;
	}

	rFile.Cursors.insert( rFile.Cursors.begin(), rCursor );

} // AddCursor

//////////////////////////////////////////////////////////////////////////

TextEdit::Cursor& TextEdit::GetLastAddedCursor( File& rFile )
{
	GENO_ASSERT( rFile.Cursors.size() > 0 );

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.Last )
			return rCursor;
	}

	GENO_ASSERT( false );

	// This should never happen
	return rFile.Cursors[ 0 ];
}

//////////////////////////////////////////////////////////////////////////

TextEdit::Cursor& TextEdit::GetMainCursor( File& rFile )
{
	GENO_ASSERT( rFile.Cursors.size() > 0 );

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.Main )
			return rCursor;
	}

	GENO_ASSERT( false );

	// This should never happen
	return rFile.Cursors[ 0 ];

} // GetMainCursor

//////////////////////////////////////////////////////////////////////////

void TextEdit::EraseAllCursors( File& rFile, bool ExcludeMainCursor )
{
	Cursor Tmp;

	if( ExcludeMainCursor )
	{
		for( Cursor& rCursor : rFile.Cursors )
		{
			if( rCursor.Main )
			{
				Tmp = rCursor;
				break;
			}
		}
	}

	rFile.Cursors.erase( rFile.Cursors.begin(), rFile.Cursors.end() );

	if( ExcludeMainCursor )
		rFile.Cursors.push_back( Tmp );

} // EraseAllCursors

//////////////////////////////////////////////////////////////////////////

int TextEdit::GetCursorIndex( File& rFile, Cursor& rCursor )
{
	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		if( rFile.Cursors[ i ].Position == rCursor.Position ) return i;
	}

	GENO_ASSERT( false );

	return -1;
} // GetCursorIndex

//////////////////////////////////////////////////////////////////////////

void TextEdit::ScrollToCursor( File& rFile )
{
	ScrollTo( rFile, rFile.Cursors.back().Position );

} // ScrollToCursor

//////////////////////////////////////////////////////////////////////////

void TextEdit::ScrollTo( File& rFile, Coordinate Position )
{
	ScrollTo( rFile, Position, ImGui::GetCurrentWindow() );
} // ScrollTo

//////////////////////////////////////////////////////////////////////////

void TextEdit::ScrollTo( File& rFile, Coordinate Position, ImGuiWindow* pWindow )
{
	ImVec2 Size = Props.WindowSize;

	float Top    = Props.ScrollY;
	float Bottom = Top + Size.y - ( Props.CharAdvanceY * 2 );
	float Left   = Props.ScrollX;
	float Right  = Left + Size.x - 10.0f;

	float CX = GetDistance( rFile, Position );
	float CY = Position.y * Props.CharAdvanceY;

	if( CY < Top )
	{
		ImGui::SetScrollY( pWindow, Position.y * Props.CharAdvanceY );
	}
	else if( CY > Bottom )
	{
		ImGui::SetScrollY( pWindow, Position.y * Props.CharAdvanceY - Size.y + ( Props.CharAdvanceY * 2 ) );
	}

	if( CX < Left )
	{
		ImGui::SetScrollX( pWindow, CX );
	}
	else if( CX > Right )
	{
		ImGui::SetScrollX( pWindow, CX - Size.x + 10.0f );
	}

} // ScrollTo

//////////////////////////////////////////////////////////////////////////

void TextEdit::CheckLineLengths( File& rFile, int FirstLine, int LastLine )
{
	for( int i = FirstLine; i <= LastLine; i++ )
	{
		Line& rLine        = rFile.Lines[ i ];
		float Length       = GetDistance( rFile, Coordinate( ( int )rLine.size(), i ) );
		int   NumLongLines = ( int )rFile.LongestLines.size();
		int   Line         = -1;
		int   j            = 0;

		for( ; j < NumLongLines; j++ )
		{
			Line = rFile.LongestLines[ j ];

			if( i == Line )
			{
				break;
			}
			else
			{
				Line = -1;
			}
		}

		if( Length > rFile.LongestLineLength )
		{
			rFile.LongestLines.clear();
			rFile.LongestLines.push_back( i );

			rFile.LongestLineLength = Length;
		}
		else if( Line != -1 )
		{
			if( Length < rFile.LongestLineLength )
			{
				rFile.LongestLines.erase( rFile.LongestLines.begin() + j );

				if( NumLongLines == 1 )
				{
					rFile.LongestLineLength = 0.0f;

					CheckLineLengths( rFile, 0, ( int )rFile.Lines.size() - 1 );
				}
			}
		}
		else if( Length == rFile.LongestLineLength )
		{
			rFile.LongestLines.push_back( i );
		}
	}

} // CheckLineLengths

//////////////////////////////////////////////////////////////////////////

float TextEdit::GetMaxCursorDistance( File& rFile )
{
	float Distance = 0.0f;

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.Disabled ) continue;

		float L = GetDistance( rFile, rCursor.Position );

		if( L > Distance ) Distance = L;
	}

	return Distance;

} // GetMaxCursorDistance

//////////////////////////////////////////////////////////////////////////

void TextEdit::CalculeteLineNumMaxWidth( File& rFile )
{
	int TotalLines = ( int )rFile.Lines.size();

	char Buf[ 32 ];
	sprintf( Buf, " %u | ", TotalLines );

	Props.LineNumMaxWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, Buf ).x;

} // CalculeteLineNumMaxWidth

//////////////////////////////////////////////////////////////////////////

bool TextEdit::HasSelection( File& rFile, int CursorIndex ) const
{
	const Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( rCursor.Disabled ) return false;

	if( rCursor.SelectionStart.y != rCursor.SelectionEnd.y )
		return rCursor.SelectionEnd.y > rCursor.SelectionStart.y;

	return rCursor.SelectionEnd.x > rCursor.SelectionStart.x;

} // HasSelection

//////////////////////////////////////////////////////////////////////////

TextEdit::Cursor* TextEdit::IsCoordinateInSelection( File& rFile, Coordinate Coordinate, int Offset )
{
	GENO_ASSERT( Offset <= ( int )rFile.Cursors.size() );

	for( int i = Offset; i < ( int )rFile.Cursors.size(); i++ )
	{
		Cursor& rCursor = rFile.Cursors[ i ];

		if( rCursor.Disabled ) continue;

		if( ( Coordinate > rCursor.SelectionStart && Coordinate < rCursor.SelectionEnd ) || ( Coordinate == rCursor.Position ) )
		{
			return &rCursor;
		}
	}

	return nullptr;

} // IsCoordinateInSelection

//////////////////////////////////////////////////////////////////////////

TextEdit::LineSelectionItem TextEdit::IsSelectionOnLine( File& rFile, int LineIndex, Coordinate Start, Coordinate End ) const
{
	LineSelectionItem Item( LineSelectionItem::None );

	if( LineIndex == Start.y )
	{
		Item.Start = Start;

		if( LineIndex == End.y )
		{
			Item.End = End;
		}
		else
		{
			Item.End.y = LineIndex;
			Item.End.x = ( int )rFile.Lines[ LineIndex ].size();
		}
	}
	else if( LineIndex >= Start.y && LineIndex <= End.y )
	{
		Item.Start.x = 0;
		Item.Start.y = LineIndex;

		if( LineIndex == End.y )
		{
			Item.End = End;
		}
		else
		{
			Item.End.y = LineIndex;
			Item.End.x = ( int )rFile.Lines[ LineIndex ].size();
		}
	}
	else
	{
		Item.Type = -1;
	}

	return Item;
}

std::vector< TextEdit::LineSelectionItem > TextEdit::IsLineSelected( File& rFile, int LineIndex ) const
{
	std::vector< LineSelectionItem > Selections;

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.SelectionStart == rCursor.SelectionEnd || rCursor.Disabled ) continue;

		LineSelectionItem Item = IsSelectionOnLine( rFile, LineIndex, rCursor.SelectionStart, rCursor.SelectionEnd );

		if( Item.Type == -1 ) continue;

		Item.Type = LineSelectionItem::Selection;

		Selections.push_back( Item );
	}

	SearchDialog& rDiag = *rFile.SearchDiag;

	if( rDiag.SearchResult )
	{
		if( rDiag.SearchResult->GroupExist( LineIndex ) )
		{
			for( LineSelectionItem* pSearch : rDiag.SearchResult->GetGroup( LineIndex ) )
			{
				LineSelectionItem Item = IsSelectionOnLine( rFile, LineIndex, pSearch->Start, pSearch->End );

				if( Item.Type == -1 ) continue;

				Item.Type = pSearch->Type;

				Selections.push_back( Item );
			}
		}
	}

	return Selections;

} // IsLineSelected

//////////////////////////////////////////////////////////////////////////

float TextEdit::GetDistance( File& rFile, Coordinate Position ) const
{
	const Line& rLine = rFile.Lines[ Position.y ];

	std::string String;

	float XOffset = 0.0f;

	int Length = Position.x > ( int )rLine.size() ? ( int )rLine.size() : Position.x;

	for( int i = 0; i < Length; i++ )
	{
		char c = rLine[ i ].C;

		if( c == '\t' )
		{
			float Tab = TabSize * Props.SpaceSize;

			XOffset += Tab;

			if( String.empty() ) continue;

			XOffset += ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, String.c_str() ).x;

			float Fraction = XOffset / Tab;
			Fraction       = Fraction - floorf( Fraction );

			XOffset -= Fraction * Tab;
			String.clear();
		}
		else
		{
			String.push_back( c );
		}
	}

	if( !String.empty() )
	{
		XOffset += ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, String.c_str() ).x;
	}

	if( int Extra = Position.x - Length )
	{
		XOffset += ( float )Extra * Props.SpaceSize;
	}

	return XOffset;

} // GetDistance

//////////////////////////////////////////////////////////////////////////

std::string TextEdit::GetWordAt( File& rFile, Cursor& rCursor ) const
{
	return GetWordAt( rFile, rCursor.Position, &rCursor.SelectionStart, &rCursor.SelectionEnd );

} // GetWordAt

//////////////////////////////////////////////////////////////////////////

std::string TextEdit::GetWordAt( File& rFile, Coordinate Position, Coordinate* pStart, Coordinate* pEnd ) const
{
	const Line& rLine    = rFile.Lines[ Position.y ];
	int         LineSize = ( int )rLine.size();

	if( LineSize == 0 )
	{
		if( pStart ) *pStart = Coordinate( 0, 0 );
		if( pEnd ) *pEnd = Coordinate( 0, 0 );

		return std::string();
	}

	if( Position.x >= LineSize )
	{
		Position.x = LineSize - 1;
	}

	char c = rLine[ Position.x ].C;

	std::string Buffer;

	auto getRegion = [ &Buffer, &rLine, &Position, pStart, pEnd ]( bool ( *cmpFunc )( char c ) ) -> std::string
	{
		int Len = ( int )rLine.size();
		int x0  = 0;
		int x1  = Len;

		for( int i = Position.x + 1; i < Len; i++ )
		{
			char Chr = rLine[ i ].C;
			if( !cmpFunc( Chr ) )
			{
				x1 = i;
				break;
			}

			Buffer.push_back( Chr );
		}

		for( int i = Position.x; i >= 0; i-- )
		{
			char Chr = rLine[ i ].C;
			if( !cmpFunc( Chr ) )
			{
				x0 = i + 1;
				break;
			}

			Buffer.insert( Buffer.begin(), Chr );
		}

		if( pStart ) *pStart = Coordinate( x0, Position.y );
		if( pEnd ) *pEnd = Coordinate( x1, Position.y );

		return std::move( Buffer );
	};

	auto cmpCharsNum = []( char c ) -> bool
	{
		return c == '_' || ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' );
	};

	auto cmpWhitespace = []( char c ) -> bool
	{
		return c == ' ' || c == '\t';
	};

	auto cmpOperators = []( char c ) -> bool
	{
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '<' || c == '>' || c == '|' || c == '&' || c == '^' || c == '!' || c == '=';
	};

	if( cmpCharsNum( c ) )
	{
		return getRegion( cmpCharsNum );
	}
	else if( cmpWhitespace( c ) )
	{
		char lc = Position.x == 0 ? ' ' : rLine[ Position.x - 1 ].C;

		if( cmpWhitespace( lc ) )
		{
			return getRegion( cmpWhitespace );
		}
		else if( cmpOperators( lc ) )
		{
			Position.x--;
			return getRegion( cmpOperators );
		}
		else if( cmpCharsNum( lc ) )
		{
			Position.x--;
			return getRegion( cmpCharsNum );
		}
		else
		{
			if( pStart ) *pStart = Coordinate( Position.x - 1, Position.y );
			if( pEnd ) *pEnd = Coordinate( Position.x, Position.y );
		}
	}
	else if( cmpOperators( c ) )
	{
		return getRegion( cmpOperators );
	}
	else
	{
		if( pStart ) *pStart = Coordinate( Position.x, Position.y );
		if( pEnd ) *pEnd = Coordinate( Position.x + 1, Position.y );

		std::string Result;
		Result.push_back( c );

		return Result;
	}

	return std::string();

} // GetWordAt

//////////////////////////////////////////////////////////////////////////

bool TextEdit::IsCoordinateInText( File& rFile, Coordinate Position )
{
	if( Position.x == 0 ) return false;

	Coordinate End;

	std::string Word = GetWordAt( rFile, Coordinate( 0, Position.y ), nullptr, &End );

	if( Word[ 0 ] == ' ' || Word[ 0 ] == '\t' )
	{
		if( Position.x <= End.x ) return false;
	}

	return true;

} // IsCoordinateInText

//////////////////////////////////////////////////////////////////////////

void TextEdit::AdjustCursorIfInText( File& rFile, Cursor& rCursor, int LineIndex, int XOffset )
{
	if( XOffset > 0 )
	{
		rCursor.Position.x += XOffset;
		rCursor.SelectionOrigin.x += XOffset;
		rCursor.SelectionStart.x += XOffset;
		rCursor.SelectionEnd.x += XOffset;
	}

	if( IsCoordinateInText( rFile, rCursor.Position ) && LineIndex == rCursor.Position.y ) rCursor.Position.x += XOffset;
	if( IsCoordinateInText( rFile, rCursor.SelectionOrigin ) && LineIndex == rCursor.SelectionOrigin.y ) rCursor.SelectionOrigin.x += XOffset;
	if( IsCoordinateInText( rFile, rCursor.SelectionStart ) && LineIndex == rCursor.SelectionStart.y ) rCursor.SelectionStart.x += XOffset;
	if( IsCoordinateInText( rFile, rCursor.SelectionEnd ) && LineIndex == rCursor.SelectionEnd.y ) rCursor.SelectionEnd.x += XOffset;

	if( XOffset > 0 )
	{
		rCursor.Position.x -= XOffset;
		rCursor.SelectionOrigin.x -= XOffset;
		rCursor.SelectionStart.x -= XOffset;
		rCursor.SelectionEnd.x -= XOffset;
	}

} // AdjustCursorIfInText

//////////////////////////////////////////////////////////////////////////

void TextEdit::AdjustCursor( Cursor& rCursor, int XOffset )
{
	rCursor.Position.x += XOffset;

	if( rCursor.SelectionOrigin != Coordinate( -1, -1 ) )
	{
		rCursor.SelectionOrigin.x += XOffset;
		rCursor.SelectionStart.x += XOffset;
		rCursor.SelectionEnd.x += XOffset;
	}

} // AdjustCursor

//////////////////////////////////////////////////////////////////////////

void TextEdit::SetSelectionLine( File& rFile, int LineIndex )
{
	if( LineIndex >= ( int )rFile.Lines.size() ) return;

	Line& rLine = rFile.Lines[ LineIndex ];

	Coordinate Start( 0, LineIndex );
	Coordinate End( ( int )rLine.size(), LineIndex );

	SetSelection( rFile, Start, End, 0 );

} // SetSelectionLine

//////////////////////////////////////////////////////////////////////////

void TextEdit::SetSelection( File& rFile, Coordinate Start, Coordinate End, int CursorIndex )
{
	GENO_ASSERT( ( ( Start.x < End.x ) || End.x == -1 ) && ( ( Start.y < End.y ) || End.y == -1 ) );
	GENO_ASSERT( CursorIndex < ( int )rFile.Cursors.size() );

	std::vector< Line >& rLines = rFile.Lines;

	if( Start.y >= ( int )rLines.size() ) return;

	if( End.y >= ( int )rLines.size() || End.y == -1 )
	{
		End.y = ( int )rLines.size() - 1;
		End.x = ( int )rLines[ End.y ].size();
	}

	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	rCursor.SelectionOrigin = Start;
	rCursor.SelectionStart  = Start;
	rCursor.SelectionEnd    = End;
	rCursor.Position        = End;

} // SetSelectionLine

//////////////////////////////////////////////////////////////////////////

void TextEdit::SelectAll( File& rFile )
{
	EraseAllCursors( rFile, true );

	rFile.CursorMultiMode = MultiCursorMode::Normal;

	SetSelection( rFile, Coordinate(), Coordinate( -1, -1 ), 0 );

} // SelectAll

//////////////////////////////////////////////////////////////////////////

int TextEdit::GetCoordinateY( File& rFile, float YPosition )
{
	int LineIndex = ( int )( ( YPosition / Props.CharAdvanceY ) );
	int NumLines  = ( int )rFile.Lines.size();

	if( LineIndex > NumLines - 1 )
	{
		LineIndex = NumLines - 1;
	}

	return LineIndex;
} // GetCoordinateY

//////////////////////////////////////////////////////////////////////////

int TextEdit::GetCoordinateX( File& rFile, int LineIndex, float XPosition, bool AllowPastLine )
{
	const Line& rLine       = rFile.Lines[ LineIndex ];
	int         LineSize    = ( int )rLine.size();
	char        String[ 2 ] = { 0, 0 };
	float       Length      = 0.0f;
	float       Diff        = 0.0f;
	bool        WithinLine  = false;
	int         Result      = -1;

	for( int i = 0; i < LineSize; i++ )
	{
		String[ 0 ] = rLine[ i ].C;
		Diff        = 0.0f;

		if( String[ 0 ] == '\t' )
		{
			float NewLen = GetDistance( rFile, Coordinate( i + 1, LineIndex ) );
			Diff         = NewLen - Length,
			Length += Diff;
		}
		else
		{
			Length += Diff = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, String ).x;
		}

		if( Length - ( Diff / 2.0f ) > XPosition )
		{
			Result     = i;
			WithinLine = true;
			break;
		}
	}

	if( WithinLine ) return Result;

	if( AllowPastLine )
	{
		Diff = Props.SpaceSize;

		for( int i = LineSize;; i++ )
		{
			Length += Diff;

			if( Length - ( Diff / 2.0f ) > XPosition )
			{
				return i;
			}
		}
	}

	return LineSize;
} // GetCoordinateX

//////////////////////////////////////////////////////////////////////////

TextEdit::Coordinate TextEdit::GetCoordinate( File& rFile, ImVec2 Position, bool AllowPastLine )
{
	int LineIndex = GetCoordinateY( rFile, Position.y );
	int XIndex    = GetCoordinateX( rFile, LineIndex, Position.x, AllowPastLine );

	return Coordinate( XIndex, LineIndex );

} // GetCoordinate

//////////////////////////////////////////////////////////////////////////

TextEdit::Coordinate TextEdit::CalculateTabAlignment( File& rFile, Coordinate FromPosition )
{
	return GetCoordinate( rFile, ImVec2( CalculateTabAlignmentDistance( rFile, FromPosition ), FromPosition.y * Props.CharAdvanceY ), false );

} // CalculateTabAlignment

//////////////////////////////////////////////////////////////////////////

float TextEdit::CalculateTabAlignmentDistance( File& rFile, Coordinate FromPosition )
{
	float Dist = GetDistance( rFile, FromPosition );

	float Tab = TabSize * Props.SpaceSize;

	float Fraction = Dist / Tab;

	if( Fraction < 1.0f ) Fraction = 1.0f;
	else
		Fraction = Fraction - floorf( Fraction );

	float NewDist = Dist - Fraction * Tab;

	if( NewDist < 0.0f ) NewDist = 0.0f;

	return NewDist;

} // CalculateTabAlignmentDistance

//////////////////////////////////////////////////////////////////////////

void TextEdit::AdjustCursors( File& rFile, int CursorIndex, int XOffset, int YOffset )
{
	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	for( int j = 0; j < ( int )rFile.Cursors.size(); j++ )
	{
		if( j == CursorIndex ) continue;

		Cursor& rOther = rFile.Cursors[ j ];

		if( rOther.SelectionStart > rCursor.SelectionStart )
		{
			if( rOther.SelectionStart.y == rCursor.SelectionStart.y )
			{
				rOther.SelectionStart.x -= XOffset;

				if( rOther.SelectionEnd.y == rCursor.SelectionStart.y )
				{
					rOther.SelectionEnd.x -= XOffset;
				}
			}
			else
			{
				rOther.SelectionStart.y -= YOffset;
				rOther.SelectionEnd.y -= YOffset;
			}
		}

		if( rOther.Position > rCursor.Position )
		{
			if( rOther.Position.y == rCursor.Position.y || YOffset > 0 ? rOther.Position.y - YOffset == rCursor.Position.y : false )
			{
				rOther.Position.x -= XOffset;
			}

			rOther.Position.y -= YOffset;
		}
	}

} // AdjustCursors

//////////////////////////////////////////////////////////////////////////

void TextEdit::YeetDuplicateCursors( File& rFile )
{
	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		Cursor& rCursor = rFile.Cursors[ i ];
		for( int j = i + 1; j < ( int )rFile.Cursors.size(); j++ )
		{
			Cursor& rTmpCursor = rFile.Cursors[ j ];

			if( rTmpCursor.Position == rCursor.Position )
			{
				rFile.Cursors.erase( rFile.Cursors.begin() + j );
			}
		}
	}

} // YeetDuplicateCursors

//////////////////////////////////////////////////////////////////////////

void TextEdit::DisableIntersectionsInSelection( File& rFile, Cursor& rCursor )
{
	rCursor.Disabled = true;

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		Cursor& rCursor2 = rFile.Cursors[ i ];

		if( rCursor2.Disabled ) continue;

		if( ( rCursor2.SelectionStart > rCursor.SelectionStart && rCursor2.SelectionStart < rCursor.SelectionEnd ) || ( rCursor2.SelectionEnd > rCursor.SelectionStart && rCursor2.SelectionEnd < rCursor.SelectionEnd ) || ( rCursor2.Position >= rCursor.SelectionStart && rCursor2.Position <= rCursor.SelectionEnd ) )
		{
			rCursor2.Disabled = true;
		}
		else
		{
			rCursor2.Disabled = false;
		}
	}

	rCursor.Disabled = false;

} // DisableIntersectingSelections

//////////////////////////////////////////////////////////////////////////

void TextEdit::DeleteDisabledCursor( File& rFile )
{
	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		if( rFile.Cursors[ i ].Disabled ) rFile.Cursors.erase( rFile.Cursors.begin() + i-- );
	}

} // DeleteDisabledCursor

//////////////////////////////////////////////////////////////////////////

void TextEdit::DeleteSelection( File& rFile, int CursorIndex )
{
	if( !HasSelection( rFile, CursorIndex ) ) return;

	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( rCursor.Disabled ) return;

	DeleteLines( rFile, CursorIndex, rCursor.SelectionStart, rCursor.SelectionEnd );

} // DeleteSelection

//////////////////////////////////////////////////////////////////////////

void TextEdit::DeleteLines( File& rFile, int CursorIndex, Coordinate Start, Coordinate End )
{
	Command*            pCommand = CursorIndex != -1 ? rFile.CmdStack.GetCommand( &rFile, CommandType::DeleteLines ) : nullptr;
	std::vector< Line > TmpLines;

	auto& rLines = rFile.Lines;

	Line& rLine = rLines[ Start.y ];

	int YOffset = 0;
	int XOffset = 0;

	if( End.y == Start.y )
	{
		int LineLength = ( int )rLine.size();

		if( Start.x > LineLength )
		{
			return;
		}
		else if( End.x > LineLength )
		{
			End.x = LineLength;
		}

		TmpLines.push_back( SubLine( rLine, Start.x, End.x ) );
		rLine.erase( rLine.begin() + Start.x, rLine.begin() + End.x );

		XOffset = End.x - Start.x;
	}
	else
	{
		TmpLines.push_back( SubLine( rLine, Start.x, ( int )rLine.size() ) );
		rLine.erase( rLine.begin() + Start.x, rLine.end() );

		int NumLines = End.y - Start.y - 1;

		if( NumLines > 0 )
		{
			for( int i = 0; i < NumLines; i++ )
			{
				TmpLines.push_back( rLines[ Start.y + 1 + i ] );
			}

			rLines.erase( rLines.begin() + Start.y + 1, rLines.begin() + Start.y + NumLines + 1 );
		}

		Line& rLine2 = rLines[ Start.y + 1 ];

		YOffset = NumLines + 1;
		XOffset = End.x;

		if( XOffset == ~0 ) XOffset++;

		TmpLines.push_back( SubLine( rLine2, 0, XOffset ) );
		rLine2.erase( rLine2.begin(), rLine2.begin() + XOffset );

		if( !rLine2.empty() )
		{
			rLine.insert( rLine.end(), rLine2.begin(), rLine2.end() );
		}

		rLines.erase( rLines.begin() + Start.y + 1 );
	}

	if( CursorIndex != -1 )
	{
		AdjustCursors( rFile, CursorIndex, XOffset, YOffset );

		SubCommand* pSub = new SubCommand( CommandType::InsertLines );

		Cursor& rCursor = rFile.Cursors[ CursorIndex ];

		pSub->CursorCopy  = rCursor;
		pSub->CursorIndex = CursorIndex;
		pSub->Lines       = std::move( TmpLines );
		pSub->NoMove      = Start == rCursor.Position;

		pCommand->AddSubCommand( pSub );

		rCursor.Position = Start;

		rCursor.SelectionStart  = { 0, 0 };
		rCursor.SelectionEnd    = { 0, 0 };
		rCursor.SelectionOrigin = { -1, -1 };
	}

	Props.Changes = true;
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::Enter( File& rFile )
{
	rFile.CmdStack.GetCommand( &rFile, CommandType::Enter );

	Props.Changes         = true;
	rFile.CursorMultiMode = MultiCursorMode::Normal;

	for( int i = 0; i < ( int )rFile.Cursors.size(); ++i )
	{
		Enter( rFile, i );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // Enter

//////////////////////////////////////////////////////////////////////////

void TextEdit::Enter( File& rFile, int CursorIndex )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::Enter );

	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( rCursor.Disabled ) return;

	DeleteSelection( rFile, CursorIndex );

	auto& rLines = rFile.Lines;

	Line  NewLine;
	Line& rLine = rLines[ rCursor.Position.y ];

	bool MoveRemainingTextToNewLine = rCursor.Position.x < ( int )rLine.size();

	if( MoveRemainingTextToNewLine )
	{
		auto Start = rLine.begin() + rCursor.Position.x;
		NewLine.insert( NewLine.begin(), Start, rLine.end() );
		rLine.erase( Start, rLine.end() );
	}

	AdjustCursors( rFile, CursorIndex, rCursor.Position.x, -1 );

	rCursor.Position.y++;
	rCursor.Position.x = 0;
	rLines.insert( rLines.begin() + rCursor.Position.y, NewLine );

	SubCommand* pSub = new SubCommand( CommandType::Backspace );

	pSub->CursorCopy  = rCursor;
	pSub->CursorIndex = CursorIndex;

	pCommand->AddSubCommand( pSub );

	Props.Changes = true;
} // Enter

//////////////////////////////////////////////////////////////////////////

void TextEdit::Backspace( File& rFile )
{
	rFile.CmdStack.GetCommand( &rFile, CommandType::Backspace );

	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		std::vector< int > CurInText = CursorsInText( rFile );

		if( ( int )CurInText.size() == 0 )
		{
			Esc( rFile );
			return;
		}
	}

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		Backspace( rFile, i, true );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

} // Backspace

//////////////////////////////////////////////////////////////////////////

void TextEdit::Backspace( File& rFile, int CursorIndex, bool DeleteLine )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::Backspace );
	Cursor&  rCursor  = rFile.Cursors[ CursorIndex ];
	bool     InText   = CursorInText( rFile, CursorIndex );

	if( rCursor.Disabled ) return;

	if( HasSelection( rFile, CursorIndex ) )
	{
		if( InText )
		{
			DeleteSelection( rFile, CursorIndex );
		}
		else
		{
			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );

			rCursor.Position.x      = rCursor.SelectionStart.x;
			rCursor.SelectionStart  = Coordinate( 0, 0 );
			rCursor.SelectionEnd    = Coordinate( 0, 0 );
			rCursor.SelectionOrigin = Coordinate( -1, -1 );
		}
	}
	else
	{
		if( InText )
		{
			auto& rLines = rFile.Lines;

			Line& rLine = rLines[ rCursor.Position.y ];

			if( rCursor.Position.x == 0 && rCursor.Position.y != 0 && DeleteLine )
			{
				Line& rLineAbove = rLines[ rCursor.Position.y - 1 ];

				int x = ( int )rLineAbove.size();

				if( !rLine.empty() )
				{
					rLineAbove.insert( rLineAbove.end(), rLine.begin(), rLine.end() );
				}

				rLines.erase( rLines.begin() + rCursor.Position.y );

				rCursor.Position.x = x;
				rCursor.Position.y--;

				SubCommand* pSub = new SubCommand( CommandType::Enter );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;

				pCommand->AddSubCommand( pSub );

				AdjustCursors( rFile, CursorIndex, -x, 1 );

				Props.Changes = true;
			}
			else if( !( rCursor.Position.y == 0 && rCursor.Position.x == 0 ) && !( rCursor.Position.x == 0 && !DeleteLine ) )
			{
				rCursor.Position.x--;

				Glyph Glyph = rLine[ rCursor.Position.x ];

				rLine.erase( rLine.begin() + rCursor.Position.x );

				AdjustCursors( rFile, CursorIndex, 1, 0 );

				SubCommand* pSub = nullptr;

				if( Glyph.C == '\t' )
				{
					pSub              = new SubCommand( CommandType::Tab );
					pSub->CursorCopy  = rCursor;
					pSub->CursorIndex = CursorIndex;
				}
				else
				{
					pSub              = new SubCommand( CommandType::EnterText );
					pSub->CursorCopy  = rCursor;
					pSub->CursorIndex = CursorIndex;
					pSub->Character   = Glyph;
				}

				pCommand->AddSubCommand( pSub );
			}
			else
			{
				return;
			}
		}
		else
		{
			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );

			if( rCursor.Position.x != 0 ) rCursor.Position.x--;
		}

		Props.Changes = true;

		YeetDuplicateCursors( rFile );
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // Backspace

/////////////////////////////////////////////////////////////////////////

void TextEdit::Del( File& rFile )
{
	rFile.CmdStack.GetCommand( &rFile, CommandType::Del );

	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		std::vector< int > CurInText = CursorsInText( rFile );

		if( ( int )CurInText.size() == 0 )
		{
			Esc( rFile );
			return;
		}
	}

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		Del( rFile, i, rFile.CursorMultiMode == MultiCursorMode::Normal );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

} // Del

//////////////////////////////////////////////////////////////////////////

void TextEdit::Del( File& rFile, int CursorIndex, bool DeleteLine )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::Del );
	Cursor&  rCursor  = rFile.Cursors[ CursorIndex ];
	bool     InText   = CursorInText( rFile, CursorIndex );

	if( rCursor.Disabled ) return;

	if( HasSelection( rFile, CursorIndex ) )
	{
		if( InText )
		{
			DeleteSelection( rFile, CursorIndex );
		}
		else
		{
			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );

			rCursor.Position.x      = rCursor.SelectionStart.x;
			rCursor.SelectionStart  = Coordinate( 0, 0 );
			rCursor.SelectionEnd    = Coordinate( 0, 0 );
			rCursor.SelectionOrigin = Coordinate( -1, -1 );
		}
	}
	else
	{
		if( InText )
		{
			Line& rLine = rFile.Lines[ rCursor.Position.y ];

			if( rCursor.Position.x >= ( int )rLine.size() && DeleteLine )
			{
				if( rCursor.Position.y == ( ( int )rFile.Lines.size() - 1 ) ) return;

				Line& rNextLine = rFile.Lines[ rCursor.Position.y + 1 ];

				rLine.insert( rLine.end(), rNextLine.begin(), rNextLine.end() );

				rFile.Lines.erase( rFile.Lines.begin() + rCursor.Position.y + 1 );

				AdjustCursors( rFile, CursorIndex, -rCursor.Position.x, 1 );

				SubCommand* pSub = new SubCommand( CommandType::Enter );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;
				pSub->NoMove      = true;

				pCommand->AddSubCommand( pSub );
			}
			else if( rCursor.Position.x < ( int )rLine.size() )
			{
				Glyph TmpGlyph = rLine[ rCursor.Position.x ];

				rLine.erase( rLine.begin() + rCursor.Position.x );
				AdjustCursors( rFile, CursorIndex, 1, 0 );

				SubCommand* pSub = new SubCommand( CommandType::EnterText );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;
				pSub->NoMove      = true;
				pSub->Character   = TmpGlyph;

				pCommand->AddSubCommand( pSub );
			}
			else
			{
				return;
			}
		}
		else
		{
			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );
		}

		Props.Changes = true;
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;
} // Del

//////////////////////////////////////////////////////////////////////////

void TextEdit::Tab( File& rFile, bool Shift )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::Tab );

	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		std::vector< int > CurInText    = CursorsInText( rFile );
		std::vector< int > CurNotInText = CursorsNotInText( rFile );

		if( ( int )CurInText.size() == 0 )
		{
			if( Shift )
			{
				for( Cursor& rCursor : rFile.Cursors )
				{
					rCursor.Position.x -= ( int )TabSize;

					if( rCursor.SelectionOrigin != Coordinate( -1, -1 ) )
					{
						rCursor.SelectionOrigin.x -= ( int )TabSize;
						rCursor.SelectionStart.x -= ( int )TabSize;
						rCursor.SelectionEnd.x -= ( int )TabSize;
					}
				}

				bool InText = CursorsInText( rFile ).size() != 0;

				if( InText )
				{
					do
					{
						for( Cursor& rCursor : rFile.Cursors )
						{
							rCursor.Position.x += 1;

							if( rCursor.SelectionOrigin != Coordinate( -1, -1 ) )
							{
								rCursor.SelectionOrigin.x += 1;
								rCursor.SelectionStart.x += 1;
								rCursor.SelectionEnd.x += 1;
							}
						}

						InText = CursorsInText( rFile ).size() != 0;
					}
					while( InText );

					for( Cursor& rCursor : rFile.Cursors )
					{
						rCursor.Position.x -= 1;

						if( rCursor.SelectionOrigin != Coordinate( -1, -1 ) )
						{
							rCursor.SelectionOrigin.x -= 1;
							rCursor.SelectionStart.x -= 1;
							rCursor.SelectionEnd.x -= 1;
						}
					}
				}
			}
			else
			{
				for( Cursor& rCursor : rFile.Cursors )
				{
					rCursor.Position.x += ( int )TabSize;

					if( rCursor.SelectionOrigin != Coordinate( -1, -1 ) )
					{
						rCursor.SelectionOrigin.x += ( int )TabSize;
						rCursor.SelectionStart.x += ( int )TabSize;
						rCursor.SelectionEnd.x += ( int )TabSize;
					}
				}
			}
		}
		else
		{
			bool AllWhitespace = true;

			if( Shift )
			{
				for( int CursorIndex : CurInText )
				{
					bool Selection = HasSelection( rFile, CursorIndex );

					Coordinate Pos = Selection ? rFile.Cursors[ CursorIndex ].SelectionStart : rFile.Cursors[ CursorIndex ].Position;

					if( Pos.x == 0 )
					{
						AllWhitespace = false;
						break;
					}

					char Whitespace = rFile.Lines[ Pos.y ][ Pos.x - 1 ].C;

					if( Whitespace != ' ' && Whitespace != '\t' )
					{
						AllWhitespace = false;
						break;
					}
				}
			}

			if( AllWhitespace )
			{
				for( int CursorIndex : CurInText )
				{
					Tab( rFile, Shift, CursorIndex );
				}

				bool Selection = HasSelection( rFile, CurInText[ 0 ] );

				Coordinate Pos = Selection ? rFile.Cursors[ CurInText[ 0 ] ].SelectionStart : rFile.Cursors[ CurInText[ 0 ] ].Position;

				float XDist = GetDistance( rFile, Pos );

				for( int CursorIndex : CurNotInText )
				{
					Cursor& rCursor = rFile.Cursors[ CursorIndex ];

					SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

					pSub->CursorCopy  = rCursor;
					pSub->CursorIndex = CursorIndex;

					pCommand->AddSubCommand( pSub );

					int NewX = GetCoordinateX( rFile, rCursor.Position.y, XDist, true );
					int Diff = NewX - ( Selection ? rCursor.SelectionStart.x : rCursor.Position.x );

					rCursor.Position.x += Diff;

					if( Selection )
					{
						rCursor.SelectionStart.x += Diff;
						rCursor.SelectionEnd.x += Diff;
						rCursor.SelectionOrigin.x += Diff;
					}
				}

				SubCommand* pSub = new SubCommand( CommandType::TabAllCursors );

				pSub->CursorIndex = -1;
				pSub->Shift       = !Shift;

				pCommand->ClearSubCommands();
				pCommand->AddSubCommand( pSub );
			}
		}
	}
	else
	{
		for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
		{
			Tab( rFile, Shift, i );
		}
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::Tab( File& rFile, bool Shift, int CursorIndex, bool IgnoreSelection )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::Tab );

	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( rCursor.Disabled ) return;

	bool Selection = HasSelection( rFile, CursorIndex );

	if( Shift )
	{
		if( Selection && rCursor.SelectionStart.y != rCursor.SelectionEnd.y )
		{
			bool DidSomething = false;

			for( int j = rCursor.SelectionStart.y; j <= rCursor.SelectionEnd.y; j++ )
			{
				Line& rLine = rFile.Lines[ j ];

				if( rLine.empty() ) continue;

				Coordinate Start;
				Coordinate End;

				std::string Word = GetWordAt( rFile, Coordinate( 0, j ), &Start, &End );

				if( Word.empty() ) continue;
				if( Word[ 0 ] != ' ' && Word[ 0 ] != '\t' ) continue;

				Coordinate NewCoord = CalculateTabAlignment( rFile, End );

				if( NewCoord == End ) NewCoord.x--;

				if( Start.x > NewCoord.x ) NewCoord.x = Start.x;

				rLine.erase( rLine.begin() + NewCoord.x, rLine.begin() + End.x );

				AdjustCursorIfInText( rFile, rCursor, j, NewCoord.x - End.x );

				DidSomething = true;
			}

			if( DidSomething )
			{
				SubCommand* pSub = new SubCommand( CommandType::Tab );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;

				pCommand->AddSubCommand( pSub );
			}

			Props.Changes = true;

			return;
		}

		Coordinate Pos = Selection ? rCursor.SelectionStart : rCursor.Position;

		if( Pos.x == 0 ) return;

		Line& rLine = rFile.Lines[ rCursor.Position.y ];

		char Chr = rLine[ Pos.x - 1 ].C;

		if( Chr != ' ' && Chr != '\t' ) return;

		Coordinate Start;
		Coordinate End;

		GetWordAt( rFile, Coordinate( Pos.x - 1, Pos.y ), &Start, &End );

		if( End != Pos )
		{
			if( End > Pos )
			{
				End = Pos;
			}
			else
			{
				Start = End;
				End   = Pos;
			}
		}

		Coordinate NewCoord = CalculateTabAlignment( rFile, End );

		if( NewCoord == End ) NewCoord.x--;

		if( Start.x > NewCoord.x ) NewCoord.x = Start.x;

		rLine.erase( rLine.begin() + NewCoord.x, rLine.begin() + End.x );

		int Offset = End.x - NewCoord.x;

		rCursor.Position.x -= Offset;

		if( Selection )
		{
			rCursor.SelectionOrigin.x -= Offset;
			rCursor.SelectionStart.x -= Offset;
			rCursor.SelectionEnd.x -= Offset;
		}

		AdjustCursors( rFile, CursorIndex, Offset, 0 );

		SubCommand* pSub = new SubCommand( CommandType::Tab );

		pSub->CursorCopy      = rCursor;
		pSub->CursorIndex     = CursorIndex;
		pSub->IgnoreSelection = true;

		pCommand->AddSubCommand( pSub );

		Props.Changes = true;
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box )
		{
			SubCommand* pSub = new SubCommand( CommandType::Tab );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;
			pSub->Shift       = true;

			pCommand->AddSubCommand( pSub );

			Coordinate Pos   = Selection ? rCursor.SelectionStart : rCursor.Position;
			Line&      rLine = rFile.Lines[ Pos.y ];

			rLine.insert( rLine.begin() + Pos.x, Glyph( '\t', m_Palette.Default ) );

			rCursor.Position.x += 1;

			if( Selection )
			{
				rCursor.SelectionStart.x += 1;
				rCursor.SelectionEnd.x += 1;
				rCursor.SelectionOrigin.x += 1;
			}
		}
		else
		{
			if( Selection && !IgnoreSelection )
			{
				if( rCursor.SelectionStart.y == rCursor.SelectionEnd.y )
				{
					DeleteSelection( rFile, CursorIndex );
				}
				else
				{
					for( int j = rCursor.SelectionStart.y; j <= rCursor.SelectionEnd.y; j++ )
					{
						Line& rLine = rFile.Lines[ j ];

						rLine.insert( rLine.begin(), Glyph( '\t', m_Palette.Default ) );

						AdjustCursorIfInText( rFile, rCursor, j, 1 );
					}

					SubCommand* pSub = new SubCommand( CommandType::Tab );

					pSub->CursorCopy  = rCursor;
					pSub->CursorIndex = CursorIndex;
					pSub->Shift       = true;

					pCommand->AddSubCommand( pSub );

					Props.Changes = true;

					return;
				}
			}

			Line& rLine = rFile.Lines[ rCursor.Position.y ];

			if( Selection )
			{
				rLine.insert( rLine.begin() + rCursor.SelectionStart.x, Glyph( '\t', m_Palette.Default ) );

				AdjustCursors( rFile, CursorIndex, -1, 0 );

				rCursor.Position.x++;
				rCursor.SelectionStart.x++;
				rCursor.SelectionEnd.x++;

				SubCommand* pSub = new SubCommand( CommandType::Tab );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;
				pSub->Shift       = true;

				pCommand->AddSubCommand( pSub );

				Props.Changes = true;

				return;
			}

			if( rFile.CursorMode == CursorInputMode::Normal )
			{
				rLine.insert( rLine.begin() + rCursor.Position.x, Glyph( '\t', m_Palette.Default ) );

				AdjustCursors( rFile, CursorIndex, -1, 0 );
			}
			else if( rFile.CursorMode == CursorInputMode::Insert )
			{
				float CurrentDist = GetDistance( rFile, rCursor.Position );
				float Tab         = TabSize * Props.SpaceSize;

				CurrentDist += Tab;

				float Fraction = CurrentDist / Tab;
				Fraction       = Fraction - floorf( Fraction );

				CurrentDist -= Tab * Fraction;

				Coordinate NewCoord = GetCoordinate( rFile, ImVec2( CurrentDist, rCursor.Position.y * Props.CharAdvanceY ), false );

				Line TmpLine = SubLine( rLine, rCursor.Position.x, NewCoord.x );

				//rLine.erase( rLine.begin() + rCursor.Position.x, rLine.begin() + NewCoord.x );
				DeleteLines( rFile, CursorIndex, rCursor.Position, NewCoord );
				rLine.insert( rLine.begin() + rCursor.Position.x, Glyph( '\t', m_Palette.Default ) );
			}

			rCursor.Position.x++;
			rCursor.SelectionOrigin = Coordinate( -1, -1 );

			SubCommand* pSub = new SubCommand( CommandType::Backspace );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );
		}

		Props.Changes = true;
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // Tab

//////////////////////////////////////////////////////////////////////////

void TextEdit::PrepareBoxModeForInput( File& rFile )
{
	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		PrepareBoxModeForInput( rFile, i );
	}
} // PrepareBoxModeForInput

//////////////////////////////////////////////////////////////////////////

void TextEdit::PrepareBoxModeForInput( File& rFile, int CursorIndex )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile );
	bool     InText   = CursorInText( rFile, CursorIndex );
	Cursor&  rCursor  = rFile.Cursors[ CursorIndex ];

	if( HasSelection( rFile, CursorIndex ) )
	{
		if( InText )
		{
			DeleteSelection( rFile, CursorIndex );
		}
		else
		{
			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = CursorIndex;

			pCommand->AddSubCommand( pSub );

			rCursor.Position.x      = rCursor.SelectionStart.x;
			rCursor.SelectionStart  = Coordinate( 0, 0 );
			rCursor.SelectionEnd    = Coordinate( 0, 0 );
			rCursor.SelectionOrigin = Coordinate( -1, -1 );
		}
	}

	if( !InText )
	{
		Line& rLine    = rFile.Lines[ rCursor.Position.y ];
		int   LineSize = ( int )rLine.size();
		int   Count    = rCursor.Position.x - LineSize;

		rLine.insert( rLine.end(), Count, Glyph( ' ', m_Palette.Default ) );

		SubCommand* pSub = new SubCommand( CommandType::Delete );

		pSub->CursorCopy  = rCursor;
		pSub->CursorIndex = CursorIndex;
		pSub->Start       = Coordinate( LineSize, rCursor.Position.y );
		pSub->End         = rCursor.Position;

		pCommand->AddSubCommand( pSub );
	}

} // PrepareBoxModeForInput

//////////////////////////////////////////////////////////////////////////

void TextEdit::EnterTextStuff( File& rFile, char C, bool NoMove )
{
	rFile.CmdStack.GetCommand( &rFile, CommandType::EnterText );

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		EnterTextStuff( rFile, C, i, NoMove );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

} // EnterTextStuff

//////////////////////////////////////////////////////////////////////////

void TextEdit::EnterTextStuff( File& rFile, char C, int CursorIndex, bool NoMove )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::EnterText );

	Props.Changes = true;

	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( rCursor.Disabled ) return;

	bool Selection = HasSelection( rFile, CursorIndex );
	bool Insert    = !Selection && rFile.CursorMode == CursorInputMode::Insert;
	bool InText    = CursorInText( rFile, CursorIndex );

	if( Selection )
	{
		if( InText )
		{
			DeleteSelection( rFile, CursorIndex );
		}
		else
		{
			PrepareBoxModeForInput( rFile, CursorIndex );
		}
	}

	if( rFile.CursorMultiMode == MultiCursorMode::Box && !InText )
	{
		PrepareBoxModeForInput( rFile, CursorIndex );
	}

	Line& rLine = rFile.Lines[ rCursor.Position.y ];
	Glyph TmpGlyph( 0, 0 );

	bool EndOfLine = rCursor.Position.x >= ( int )rLine.size();

	if( EndOfLine )
	{
		rLine.push_back( Glyph( C, m_Palette.Default ) );
	}
	else if( Insert )
	{
		TmpGlyph = rLine[ rCursor.Position.x ];

		rLine[ rCursor.Position.x ].C = C;
	}
	else
	{
		rLine.insert( rLine.begin() + rCursor.Position.x, Glyph( C, m_Palette.Default ) );

		AdjustCursors( rFile, CursorIndex, -1, 0 );
	}

	if( !NoMove )
		rCursor.Position.x++;

	SubCommand* pSub = new SubCommand( CommandType::Backspace );

	pSub->CursorCopy  = rCursor;
	pSub->CursorIndex = CursorIndex;

	if( NoMove )
	{
		pSub->Type   = CommandType::Del;
		pSub->NoMove = true;
	}

	if( Insert )
	{
		pSub->Type = CommandType::EnterText;
		if( !NoMove ) pSub->CursorCopy.Position.x--;
		pSub->Character = TmpGlyph;
		pSub->NoMove    = true;
	}

	pCommand->AddSubCommand( pSub );

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // EnterTextStuff

//////////////////////////////////////////////////////////////////////////

void TextEdit::MoveUp( File& rFile, bool Shift, bool Alt )
{
	if( Alt && !Shift )
	{
		SwapLines( rFile, true );
		return;
	}

	if( ( rFile.CursorMultiMode == MultiCursorMode::Box || Alt ) && Shift )
	{
		Cursor& rRefCursor = GetMainCursor( rFile );

		if( rFile.CursorMultiMode == MultiCursorMode::Normal )
		{
			Esc( rFile );
		}

		if( rRefCursor.SelectionOrigin == Coordinate( -1, -1 ) )
		{
			rRefCursor.SelectionOrigin = rRefCursor.Position;
		}

		rFile.CursorMultiMode = MultiCursorMode::Box;

		Coordinate Position;

		switch( rFile.BoxModeDir )
		{
			case BoxModeDirection::Up:
				Position = rFile.Cursors[ 0 ].Position;
				break;
			case BoxModeDirection::Down:
				Position = rFile.Cursors.back().Position;
				break;
			case BoxModeDirection::None:
				Position = rRefCursor.Position;
		}

		float XDist = GetDistance( rFile, Position );
		int   Line  = Position.y - 1;

		if( Line < 0 ) return;

		SetBoxSelection( rFile, Line, XDist );
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box )
		{
			Esc( rFile );
		}

		for( Cursor& rCursor : rFile.Cursors )
		{
			if( rCursor.Position.y == 0 ) continue;

			if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
			{
				rCursor.SelectionEnd = rCursor.SelectionOrigin = rCursor.Position;
			}

			rCursor.Position.y--;

			Line& rLine = rFile.Lines[ rCursor.Position.y ];

			if( rCursor.Position.x > ( int )rLine.size() ) rCursor.Position.x = ( int )rLine.size();

			if( Shift )
			{
				if( rCursor.Position < rCursor.SelectionOrigin )
				{
					rCursor.SelectionStart = rCursor.Position;
					rCursor.SelectionEnd   = rCursor.SelectionOrigin;
				}
				else
				{
					rCursor.SelectionEnd   = rCursor.Position;
					rCursor.SelectionStart = rCursor.SelectionOrigin;
				}

				Cursor* pChamp = &rCursor;
				int     Offset = 0;

				while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
				{
					if( pOther == pChamp )
					{
						Offset++;
						continue;
					}
					pOther->SelectionEnd = pOther->SelectionOrigin = pChamp->SelectionEnd;

					pChamp->Disabled = true;
					pChamp           = pOther;
				}

				continue;
			}

			rCursor.SelectionStart = rCursor.SelectionEnd = { 0, 0 };
			rCursor.SelectionOrigin                       = { -1, -1 };
		}

		DeleteDisabledCursor( rFile );
		YeetDuplicateCursors( rFile );
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // MoveUp

//////////////////////////////////////////////////////////////////////////

void TextEdit::MoveDown( File& rFile, bool Shift, bool Alt )
{
	if( Alt && !Shift )
	{
		SwapLines( rFile, false );
		return;
	}

	if( ( rFile.CursorMultiMode == MultiCursorMode::Box || Alt ) && Shift )
	{
		Cursor& rRefCursor = GetMainCursor( rFile );

		if( rFile.CursorMultiMode == MultiCursorMode::Normal )
		{
			Esc( rFile );
		}

		if( rRefCursor.SelectionOrigin == Coordinate( -1, -1 ) )
		{
			rRefCursor.SelectionOrigin = rRefCursor.Position;
		}

		rFile.CursorMultiMode = MultiCursorMode::Box;

		Coordinate Position;

		switch( rFile.BoxModeDir )
		{
			case BoxModeDirection::Up:
				Position = rFile.Cursors[ 0 ].Position;
				break;
			case BoxModeDirection::Down:
				Position = rFile.Cursors.back().Position;
				break;
			case BoxModeDirection::None:
				Position = rRefCursor.Position;
		}

		float XDist = GetDistance( rFile, Position );
		int   Line  = Position.y + 1;

		if( Line >= ( int )rFile.Lines.size() ) return;

		SetBoxSelection( rFile, Line, XDist );
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box )
		{
			Esc( rFile );
		}

		for( Cursor& rCursor : rFile.Cursors )
		{
			if( rCursor.Position.y == ( int )rFile.Lines.size() - 1 ) continue;

			if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
			{
				rCursor.SelectionStart = rCursor.SelectionOrigin = rCursor.Position;
			}

			rCursor.Position.y++;

			Line& rLine = rFile.Lines[ rCursor.Position.y ];

			if( rCursor.Position.x > ( int )rLine.size() ) rCursor.Position.x = ( int )rLine.size();

			if( Shift )
			{
				if( rCursor.Position < rCursor.SelectionOrigin )
				{
					rCursor.SelectionStart = rCursor.Position;
					rCursor.SelectionEnd   = rCursor.SelectionOrigin;
				}
				else
				{
					rCursor.SelectionEnd   = rCursor.Position;
					rCursor.SelectionStart = rCursor.SelectionOrigin;
				}

				Cursor* pChamp = &rCursor;
				int     Offset = 0;

				while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
				{
					if( pOther == pChamp )
					{
						Offset++;
						continue;
					}
					pOther->SelectionStart = pOther->SelectionOrigin = pChamp->SelectionStart;

					pChamp->Disabled = true;
					pChamp           = pOther;
				}

				continue;
			}

			rCursor.SelectionStart = rCursor.SelectionEnd = { 0, 0 };
			rCursor.SelectionOrigin                       = { -1, -1 };
		}

		DeleteDisabledCursor( rFile );
		YeetDuplicateCursors( rFile );
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // MoveDown

//////////////////////////////////////////////////////////////////////////

void TextEdit::MoveRight( File& rFile, bool Ctrl, bool Shift, bool Alt )
{
	if( ( rFile.CursorMultiMode == MultiCursorMode::Box || Alt ) && Shift )
	{
		Cursor& rCursor = GetMainCursor( rFile );

		if( rFile.CursorMultiMode == MultiCursorMode::Normal )
		{
			Esc( rFile );
		}

		if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) )
		{
			rCursor.SelectionOrigin = rCursor.Position;
		}

		rFile.CursorMultiMode = MultiCursorMode::Box;

		Coordinate Coord;

		switch( rFile.BoxModeDir )
		{
			case BoxModeDirection::Up:
				Coord = rFile.Cursors[ 0 ].Position;
				break;
			case BoxModeDirection::Down:
				Coord = rFile.Cursors.back().Position;
				break;
			case BoxModeDirection::None:
				Coord = rCursor.Position;
		}

		if( Ctrl )
		{
			if( Coord.x == ( int )rFile.Lines[ Coord.y ].size() )
			{
				if( Coord.y < ( int )rFile.Lines.size() - 1 )
				{
					Coord.x = 0;
					Coord.y++;
				}
			}

			Coordinate Start;
			Coordinate End;
			GetWordAt( rFile, Coord, &Start, &End );

			if( Coord.x == End.x ) Coord.x++;
			else
				Coord.x = End.x;
		}
		else
		{
			Coord.x += 1;
		}

		float XDist = GetDistance( rFile, Coord );

		SetBoxSelection( rFile, Coord.y, XDist );
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box )
		{
			End( rFile, false, false );
		}

		for( Cursor& rCursor : rFile.Cursors )
		{
			if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
			{
				rCursor.SelectionStart = rCursor.SelectionOrigin = rCursor.Position;
			}

			Line& rLine = rFile.Lines[ rCursor.Position.y ];

			if( rCursor.Position.x == ( int )rLine.size() )
			{
				if( rCursor.Position.y == ( int )rFile.Lines.size() - 1 ) continue;

				rCursor.Position.x = 0;
				rCursor.Position.y++;
				Ctrl = false;
			}
			else if( !Ctrl )
			{
				rCursor.Position.x++;
			}

			if( Ctrl )
			{
				Coordinate Start;
				Coordinate End;
				GetWordAt( rFile, rCursor.Position, &Start, &End );

				if( rCursor.Position.x == End.x ) rCursor.Position.x++;
				else
					rCursor.Position.x = End.x;
			}

			if( Shift )
			{
				if( rCursor.Position > rCursor.SelectionOrigin )
				{
					rCursor.SelectionStart = rCursor.SelectionOrigin;
					rCursor.SelectionEnd   = rCursor.Position;
				}
				else
				{
					rCursor.SelectionEnd   = rCursor.SelectionOrigin;
					rCursor.SelectionStart = rCursor.Position;
				}

				Cursor* pChamp = &rCursor;
				int     Offset = 0;

				while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
				{
					if( pOther == pChamp )
					{
						Offset++;
						continue;
					}

					pOther->SelectionStart = pOther->SelectionOrigin = pChamp->SelectionStart;

					pChamp->Disabled = true;
					pChamp           = pOther;
				}

				continue;
			}

			rCursor.SelectionStart = rCursor.SelectionEnd = { 0, 0 };
			rCursor.SelectionOrigin                       = { -1, -1 };
		}

		DeleteDisabledCursor( rFile );
		YeetDuplicateCursors( rFile );
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // MoveRight

//////////////////////////////////////////////////////////////////////////

void TextEdit::MoveLeft( File& rFile, bool Ctrl, bool Shift, bool Alt )
{
	if( ( rFile.CursorMultiMode == MultiCursorMode::Box || Alt ) && Shift )
	{
		Cursor& rCursor = GetMainCursor( rFile );

		if( rFile.CursorMultiMode == MultiCursorMode::Normal )
		{
			Esc( rFile );
		}

		if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) )
		{
			rCursor.SelectionOrigin = rCursor.Position;
		}

		rFile.CursorMultiMode = MultiCursorMode::Box;

		Coordinate Coord;

		switch( rFile.BoxModeDir )
		{
			case BoxModeDirection::Up:
				Coord = rFile.Cursors[ 0 ].Position;
				break;
			case BoxModeDirection::Down:
				Coord = rFile.Cursors.back().Position;
				break;
			case BoxModeDirection::None:
				Coord = rCursor.Position;
		}

		if( Coord.x == 0 && Coord.y == 0 ) return;

		if( Ctrl )
		{
			if( Coord.x == 0 )
			{
				Coord.y--;
				Coord.x = ( int )rFile.Lines[ Coord.y ].size();
			}

			Coordinate Start;
			Coordinate End;
			GetWordAt( rFile, Coord, &Start, &End );

			if( Coord.x == Start.x )
			{
				if( Coord.x != 0 )
				{
					Coord.x--;
					GetWordAt( rFile, Coord, &Start, &End );

					Coord.x = Start.x;
				}
			}
			else
			{
				Coord.x = Start.x;
			}
		}
		else
		{
			Coord.x -= 1;
		}

		float XDist = GetDistance( rFile, Coord );

		SetBoxSelection( rFile, Coord.y, XDist );
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box )
		{
			End( rFile, false, false );
		}

		for( Cursor& rCursor : rFile.Cursors )
		{
			if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
			{
				rCursor.SelectionEnd = rCursor.SelectionOrigin = rCursor.Position;
			}

			int LineSize = -1;

			if( rCursor.Position.x == 0 && rCursor.Position.y != 0 )
			{
				Line& rLine        = rFile.Lines[ --rCursor.Position.y ];
				rCursor.Position.x = LineSize = ( int )rLine.size();
			}
			else if( !Ctrl )
			{
				rCursor.Position.x--;
			}

			if( Ctrl && LineSize == -1 )
			{
				Coordinate Start;
				Coordinate End;
				GetWordAt( rFile, rCursor.Position, &Start, &End );

				if( rCursor.Position.x == Start.x )
				{
					if( rCursor.Position.x != 0 )
					{
						rCursor.Position.x--;
						GetWordAt( rFile, rCursor.Position, &Start, &End );

						rCursor.Position.x = Start.x;
					}
				}
				else
				{
					rCursor.Position.x = Start.x;
				}
			}

			if( Shift )
			{
				if( rCursor.Position > rCursor.SelectionOrigin )
				{
					rCursor.SelectionStart = rCursor.SelectionOrigin;
					rCursor.SelectionEnd   = rCursor.Position;
				}
				else
				{
					rCursor.SelectionEnd   = rCursor.SelectionOrigin;
					rCursor.SelectionStart = rCursor.Position;
				}

				Cursor* pChamp = &rCursor;
				int     Offset = 0;

				while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
				{
					if( pOther == pChamp )
					{
						Offset++;
						continue;
					}

					pOther->SelectionEnd = pOther->SelectionOrigin = pChamp->SelectionEnd;

					pChamp->Disabled = true;
					pChamp           = pOther;
				}

				continue;
			}

			rCursor.SelectionStart = rCursor.SelectionEnd = { 0, 0 };
			rCursor.SelectionOrigin                       = { -1, -1 };
		}

		DeleteDisabledCursor( rFile );
		YeetDuplicateCursors( rFile );
	}

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // MoveLeft

//////////////////////////////////////////////////////////////////////////

void TextEdit::Home( File& rFile, bool Ctrl, bool Shift )
{
	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		Esc( rFile );
	}

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.Disabled ) continue;

		if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
		{
			rCursor.SelectionEnd = rCursor.SelectionOrigin = rCursor.Position;
		}

		Coordinate NewPos( 0, rCursor.Position.y );

		if( !Ctrl )
		{
			Coordinate  End;
			std::string Word = GetWordAt( rFile, NewPos, nullptr, &End );

			if( !Word.empty() )
			{
				if( ( Word[ 0 ] == ' ' || Word[ 0 ] == '\t' ) && End != rCursor.Position )
				{
					NewPos = End;
				}
			}
		}
		else
		{
			NewPos.y = 0;
		}

		if( Shift )
		{
			rCursor.SelectionEnd = rCursor.SelectionOrigin;
			rCursor.Position = rCursor.SelectionStart = NewPos;

			Cursor* pChamp = &rCursor;
			int     Offset = 0;

			while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
			{
				if( pOther == pChamp )
				{
					Offset++;
					continue;
				}

				if( pChamp->SelectionEnd > pOther->SelectionEnd )
				{
					pOther->SelectionEnd = pOther->SelectionOrigin = pChamp->SelectionEnd;
					pChamp->Disabled                               = true;
					pChamp                                         = pOther;
				}
				else
				{
					pChamp->Disabled = true;
					pChamp           = pOther;
				}
			}

			continue;
		}

		rCursor.Position       = NewPos;
		rCursor.SelectionStart = rCursor.SelectionEnd = Coordinate( 0, 0 );
		rCursor.SelectionOrigin                       = Coordinate( -1, -1 );
	}

	DeleteDisabledCursor( rFile );
	YeetDuplicateCursors( rFile );

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // Home

//////////////////////////////////////////////////////////////////////////

void TextEdit::End( File& rFile, bool Ctrl, bool Shift )
{
	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		Esc( rFile );
	}

	for( Cursor& rCursor : rFile.Cursors )
	{
		if( rCursor.Disabled ) continue;

		if( rCursor.SelectionOrigin == Coordinate( -1, -1 ) && Shift )
		{
			rCursor.SelectionStart = rCursor.SelectionOrigin = rCursor.Position;
		}

		Coordinate NewPos( 0, Ctrl ? ( int )rFile.Lines.size() - 1 : rCursor.Position.y );

		NewPos.x = ( int )rFile.Lines[ NewPos.y ].size();

		if( Shift )
		{
			rCursor.SelectionStart = rCursor.SelectionOrigin;
			rCursor.Position = rCursor.SelectionEnd = NewPos;

			Cursor* pChamp = &rCursor;
			int     Offset = 0;

			while( Cursor* pOther = IsCoordinateInSelection( rFile, pChamp->Position, Offset ) )
			{
				if( pOther == pChamp )
				{
					Offset++;
					continue;
				}

				if( pChamp->SelectionStart < pOther->SelectionStart )
				{
					pOther->SelectionStart = pOther->SelectionOrigin = pChamp->SelectionStart;
					pChamp->Disabled                                 = true;
					pChamp                                           = pOther;
				}
				else
				{
					pChamp->Disabled = true;
					pChamp           = pOther;
				}
			}

			continue;
		}

		rCursor.Position       = NewPos;
		rCursor.SelectionStart = rCursor.SelectionEnd = Coordinate( 0, 0 );
		rCursor.SelectionOrigin                       = Coordinate( -1, -1 );
	}

	DeleteDisabledCursor( rFile );
	YeetDuplicateCursors( rFile );

	ScrollToCursor( rFile );

	Props.CursorBlink = 0;

} // End

//////////////////////////////////////////////////////////////////////////

void TextEdit::Esc( File& rFile )
{
	EraseAllCursors( rFile, true );

	Cursor& rCursor         = rFile.Cursors[ 0 ];
	rCursor.SelectionStart  = Coordinate( 0, 0 );
	rCursor.SelectionEnd    = Coordinate( 0, 0 );
	rCursor.SelectionOrigin = Coordinate( -1, -1 );

	rFile.CursorMultiMode        = MultiCursorMode::Normal;
	rFile.SearchDiag->Searching  = false;
	rFile.SearchDiag->ActiveItem = -1;

	delete rFile.CmdStack.pCurrentCommand;
	rFile.CmdStack.pCurrentCommand = nullptr;

	ClearSearch( rFile );
} // Esc

//////////////////////////////////////////////////////////////////////////

void TextEdit::Copy( File& rFile, bool Cut )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile );

	std::string ClipBuffer;

	for( size_t i = 0; i < rFile.Cursors.size(); i++ )
	{
		Cursor& rCursor = rFile.Cursors[ i ];

		if( rCursor.Disabled ) continue;

		if( HasSelection( rFile, ( int )i ) )
		{
			if( rCursor.SelectionStart.y == rCursor.SelectionEnd.y )
			{
				Line&       rLine = rFile.Lines[ rCursor.SelectionStart.y ];
				std::string Text  = GetString( rLine, rCursor.SelectionStart.x, rCursor.SelectionEnd.x );

				ClipBuffer.append( Text + "\n" );
			}
			else
			{
				Line&       rFirstLine = rFile.Lines[ rCursor.SelectionStart.y ];
				std::string Text       = GetString( rFirstLine, rCursor.SelectionStart.x, ( int )rFirstLine.size() );

				ClipBuffer.append( Text + "\n" );

				for( int j = rCursor.SelectionStart.y + 1; j <= rCursor.SelectionEnd.y - 1; j++ )
				{
					Line& rLine = rFile.Lines[ j ];

					if( rLine.empty() )
					{
						ClipBuffer.push_back( '\n' );
						continue;
					}

					Text = GetString( rLine, 0, ( int )rLine.size() );

					ClipBuffer.append( Text + "\n" );
				}

				Line& rLastLine = rFile.Lines[ rCursor.SelectionEnd.y ];
				Text            = GetString( rLastLine, 0, rCursor.SelectionEnd.x );

				ClipBuffer.append( Text + "\n" );
			}

			if( Cut ) DeleteSelection( rFile, ( int )i );
		}
		else if( rFile.Cursors.size() == 1 )
		{
			Line&       rLine = rFile.Lines[ rCursor.Position.y ];
			std::string Text  = GetString( rLine, 0, ( int )rLine.size() );

			SubCommand* pSub = new SubCommand( CommandType::RestoreCursor );

			pSub->CursorCopy  = rCursor;
			pSub->CursorIndex = ( int )i;

			pCommand->AddSubCommand( pSub );

			if( Cut ) DeleteLines( rFile, ( int )i, Coordinate( 0, rCursor.Position.y ), Coordinate( 0, rCursor.Position.y + 1 ) ); //rFile.Lines.erase( rFile.Lines.begin() + rCursor.Position.y );

			ClipBuffer.append( Text + "\n" );
		}
		else if( Cut )
		{
			Del( rFile, ( int )i, true );
		}
	}

	if( !ClipBuffer.empty() )
	{
		ClipBuffer.erase( ClipBuffer.end() - 1 );
		ImGui::SetClipboardText( ClipBuffer.c_str() );
	}
	else
	{
		ImGui::SetClipboardText( "" );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

} // Copy

//////////////////////////////////////////////////////////////////////////

void TextEdit::Paste( File& rFile )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile );

	std::string ClipBoard( ImGui::GetClipboardText() );

	if( ClipBoard.empty() ) return;

	int                 NumLinesInClipboard = 0;
	int                 NumCursors          = ( int )rFile.Cursors.size();
	std::vector< Line > ClipLines           = SplitLines( ClipBoard, &NumLinesInClipboard );

	if( rFile.CursorMultiMode == MultiCursorMode::Box )
	{
		PrepareBoxModeForInput( rFile );

		if( NumCursors >= NumLinesInClipboard )
		{
			NumCursors--;
			for( int i = 0; i < NumLinesInClipboard; i++ )
			{
				int     CursorIndex = rFile.BoxModeDir == BoxModeDirection::Down ? i : NumCursors - i;
				Cursor& rCursor     = rFile.Cursors[ CursorIndex ];
				Line&   rLine       = rFile.Lines[ rCursor.Position.y ];
				Line&   rClipLine   = ClipLines[ i ];

				rLine.insert( rLine.begin() + rCursor.Position.x, rClipLine.begin(), rClipLine.end() );

				SubCommand* pSub = new SubCommand( CommandType::DeleteLines );

				pSub->CursorCopy  = rCursor;
				pSub->CursorIndex = CursorIndex;
				pSub->Start       = rCursor.Position;
				pSub->End         = Coordinate( rCursor.Position.x + ( int )rClipLine.size(), rCursor.Position.y );

				pCommand->AddSubCommand( pSub );
			}

			Props.Changes = true;

			rFile.CmdStack.FinishCommand( rFile.Cursors );

			return;
		}
	}

	for( int i = 0; i < NumCursors; i++ )
	{
		Cursor& rCursor = rFile.Cursors[ i ];

		if( rCursor.Disabled ) continue;

		if( HasSelection( rFile, ( int )i ) )
		{
			DeleteSelection( rFile, ( int )i );
		}

		InsertLines( rFile, i, ClipLines );
	}

	rFile.CmdStack.FinishCommand( rFile.Cursors );

	ScrollToCursor( rFile );

} // Paste

//////////////////////////////////////////////////////////////////////////

void TextEdit::InsertLines( File& rFile, int CursorIndex, std::vector< Line >& rLines, InsertLineMode Mode )
{
	Cursor& rCursor    = rFile.Cursors[ CursorIndex ];
	Line&   rFirstLine = rFile.Lines[ rCursor.Position.y ];
	int     NumLines   = ( int )rLines.size();

	Command*    pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::InsertLines );
	SubCommand* pSub     = new SubCommand( CommandType::DeleteLines );

	pSub->CursorIndex = CursorIndex;
	pSub->CursorCopy  = rCursor;

	if( rFile.CursorMultiMode == MultiCursorMode::Box )
		PrepareBoxModeForInput( rFile, CursorIndex );

	rFirstLine.insert( rFirstLine.begin() + rCursor.Position.x, rLines[ 0 ].begin(), rLines[ 0 ].end() );

	if( NumLines > 1 )
	{
		int   XOffset        = rCursor.Position.x + ( int )rLines[ 0 ].size();
		Line& rLastClipLine  = rLines.back();
		Line  OgLastClipLine = rLastClipLine;

		rLastClipLine.insert( rLastClipLine.end(), rFirstLine.begin() + XOffset, rFirstLine.end() );
		rFirstLine.erase( rFirstLine.begin() + XOffset, rFirstLine.end() );

		rFile.Lines.insert( rFile.Lines.begin() + rCursor.Position.y + 1, rLines.begin() + 1, rLines.end() );

		rLines.back() = OgLastClipLine;

		AdjustCursors( rFile, ( int )CursorIndex, XOffset, -( NumLines - 1 ) );

		if( Mode == InsertLineMode::Default )
		{
			rCursor.Position.x = ( int )OgLastClipLine.size();
			rCursor.Position.y += NumLines - 1;
		}
		else
		{
			rCursor.SelectionStart = Coordinate( rCursor.Position.x, rCursor.Position.y );
			rCursor.SelectionEnd   = Coordinate( ( int )OgLastClipLine.size(), rCursor.Position.y + NumLines - 1 );
		}
	}
	else
	{
		int XOffset = ( int )rLines[ 0 ].size();

		AdjustCursors( rFile, ( int )CursorIndex, -XOffset, 0 );

		if( Mode == InsertLineMode::Default )
		{
			rCursor.Position.x += XOffset;
		}
		else
		{
			rCursor.SelectionStart = Coordinate( rCursor.Position.x, rCursor.Position.y );
			rCursor.SelectionEnd   = Coordinate( rCursor.Position.x + XOffset, rCursor.Position.y );
		}
	}

	switch( Mode )
	{
		case InsertLineMode::SelectionStart:
			rCursor.Position        = rCursor.SelectionStart;
			rCursor.SelectionOrigin = rCursor.SelectionEnd;
			pSub->Start             = rCursor.SelectionStart;
			pSub->End               = rCursor.SelectionEnd;
			break;
		case InsertLineMode::SelectionEnd:
			rCursor.Position        = rCursor.SelectionEnd;
			rCursor.SelectionOrigin = rCursor.SelectionStart;
			pSub->Start             = rCursor.SelectionStart;
			pSub->End               = rCursor.SelectionEnd;
			break;
		case InsertLineMode::Start:
			pSub->Start            = rCursor.SelectionStart;
			pSub->End              = rCursor.SelectionEnd;
			rCursor.SelectionStart = Coordinate( 0, 0 );
			rCursor.SelectionEnd   = Coordinate( 0, 0 );
			break;
		case InsertLineMode::Default:
			pSub->Start = pSub->CursorCopy.Position;
			pSub->End   = rCursor.Position;
			break;
	}

	pCommand->AddSubCommand( pSub );

	Props.Changes = true;

} // InsertLines

//////////////////////////////////////////////////////////////////////////

void TextEdit::InsertLines( File& rFile, Cursor& rCursor, std::vector< Line >& rLines, InsertLineMode Mode )
{
	InsertLines( rFile, GetCursorIndex( rFile, rCursor ), rLines, Mode );

} // InsertLines

//////////////////////////////////////////////////////////////////////////

void TextEdit::SwapLines( File& rFile, bool Up )
{
	Command* pCommand = rFile.CmdStack.GetCommand( &rFile, CommandType::SwapLines );

	if( rFile.Cursors.size() > 1 && rFile.CursorMultiMode == MultiCursorMode::Normal ) EraseAllCursors( rFile, true );

	int     LineToMove  = -1;
	int     Destination = -1;
	bool    Selection   = HasSelection( rFile, 0 );
	Cursor& rCursor     = rFile.Cursors[ 0 ];
	Cursor& rBackCursor = rFile.Cursors.back();

	if( Up )
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box && rFile.BoxModeDir != BoxModeDirection::None )
		{
			LineToMove  = rCursor.Position.y - 1;
			Destination = rBackCursor.Position.y + 1;

			if( LineToMove == -1 ) return;

			for( Cursor& rCur : rFile.Cursors )
			{
				rCur.Position.y--;

				if( Selection )
				{
					rCur.SelectionStart.y--;
					rCur.SelectionEnd.y--;
					rCur.SelectionOrigin.y--;
				}
			}

			Selection = true;
		}
		else
		{
			if( Selection )
			{
				LineToMove  = rCursor.SelectionStart.y - 1;
				Destination = rCursor.SelectionEnd.y + 1;
			}
			else
			{
				LineToMove  = rCursor.Position.y - 1;
				Destination = rCursor.Position.y + 1;
			}

			if( LineToMove == -1 ) return;

			rCursor.Position.y--;

			if( Selection )
			{
				rCursor.SelectionStart.y--;
				rCursor.SelectionEnd.y--;
				rCursor.SelectionOrigin.y--;
			}
		}
	}
	else
	{
		if( rFile.CursorMultiMode == MultiCursorMode::Box && rFile.BoxModeDir != BoxModeDirection::None )
		{
			LineToMove  = rBackCursor.Position.y + 1;
			Destination = rCursor.Position.y;

			if( LineToMove + 1 == ( int )rFile.Lines.size() ) return;

			for( Cursor& rCur : rFile.Cursors )
			{
				rCur.Position.y++;

				if( Selection )
				{
					rCur.SelectionStart.y++;
					rCur.SelectionEnd.y++;
					rCur.SelectionOrigin.y++;
				}
			}

			Selection = true;
		}
		else
		{
			if( Selection )
			{
				LineToMove  = rCursor.SelectionEnd.y + 1;
				Destination = rCursor.SelectionStart.y;
			}
			else
			{
				LineToMove  = rCursor.Position.y;
				Destination = rCursor.Position.y + 2;
			}

			if( LineToMove + 1 == ( int )rFile.Lines.size() ) return;

			rCursor.Position.y++;

			if( Selection )
			{
				rCursor.SelectionStart.y++;
				rCursor.SelectionEnd.y++;
				rCursor.SelectionOrigin.y++;
			}
		}
	}

	auto& rLines = rFile.Lines;

	rLines.insert( rLines.begin() + Destination, rLines[ LineToMove ] );
	rLines.erase( rLines.begin() + LineToMove + ( ( !Up && Selection ) ? 1 : 0 ) );

	SubCommand* pSub = new SubCommand( CommandType::SwapLines );

	pSub->Up = !Up;

	pCommand->AddSubCommand( pSub );

	rFile.CmdStack.FinishCommand( rFile.Cursors );

	ScrollToCursor( rFile );

	Props.Changes = true;

} // SwapLines

//////////////////////////////////////////////////////////////////////////

std::vector< int > TextEdit::CursorsInText( File& rFile )
{
	std::vector< int > Cursors;

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		if( CursorInText( rFile, i ) )
		{
			Cursors.push_back( i );
		}
	}

	return Cursors;
} // CursorsInText

//////////////////////////////////////////////////////////////////////////

std::vector< int > TextEdit::CursorsNotInText( File& rFile )
{
	std::vector< int > Cursors;

	for( int i = 0; i < ( int )rFile.Cursors.size(); i++ )
	{
		if( !CursorInText( rFile, i ) )
		{
			Cursors.push_back( i );
		}
	}

	return Cursors;
} // CursorsNotInText

//////////////////////////////////////////////////////////////////////////

bool TextEdit::CursorInText( File& rFile, int CursorIndex )
{
	Cursor& rCursor = rFile.Cursors[ CursorIndex ];

	if( HasSelection( rFile, CursorIndex ) )
	{
		if( rCursor.SelectionStart < rCursor.SelectionEnd )
		{
			if( rCursor.SelectionStart.x <= ( int )rFile.Lines[ rCursor.SelectionStart.y ].size() ) return true;
		}
		else
		{
			if( rCursor.SelectionEnd.x <= ( int )rFile.Lines[ rCursor.SelectionStart.y ].size() ) return true;
			;
		}
	}
	else if( rCursor.Position.x <= ( int )rFile.Lines[ rCursor.Position.y ].size() )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::ClearSearch( File& rFile )
{
	delete rFile.SearchDiag->SearchResult;
	rFile.SearchDiag->SearchResult = nullptr;

	StatusBar::Instance().SetSearchResultInfo( "", 0 );
} // ClearSearch

//////////////////////////////////////////////////////////////////////////

TextEdit::Coordinate TextEdit::SearchInLine( File& rFile, bool CaseSensitive, const std::string& rSearchString, Coordinate Start, int SearchStringOffset, std::vector< Glyph* >& rMatches )
{
	Line& rLine = rFile.Lines[ Start.y ];

	int OgX = Start.x;

	if( OgX > ( int )rLine.size() ) return Start;

	for( int i = SearchStringOffset; i < ( int )rSearchString.length(); i++ )
	{
		char C     = rSearchString[ i ];
		int  Index = OgX + ( i - SearchStringOffset );
		bool EOL   = Index == ( int )rLine.size();

		if( EOL )
		{
			return Coordinate( OgX, Start.y );
		}

		Glyph* pGlyph = &rLine[ Index ];

		char GC = pGlyph->C;

		if( !CaseSensitive )
		{
			GC = ( char )std::tolower( GC );
			C  = ( char )std::tolower( C );
		}

		if( GC != C )
		{
			return Coordinate( OgX, Start.y );
		}

		rMatches.push_back( pGlyph );

		Start.x++;
	}

	return Start;

} // SearchInLine

//////////////////////////////////////////////////////////////////////////

void TextEdit::SearchWorker( File* pFile, bool CaseSensitive, bool WholeWord, const std::string* pSearchString, int StartLine, int EndLine, std::vector< LineSelectionItem >* pResult, int* pState )
{
	Coordinate            Start( 0, StartLine );
	std::vector< Glyph* > Matches;

	while( *pState == SearchInstance::Running )
	{
		Coordinate Next = SearchInLine( *pFile, CaseSensitive, *pSearchString, Start, 0, Matches );

		if( Next == Start )
		{
			const Line& rLine = pFile->Lines[ Start.y ];

			if( Start.x >= ( int )rLine.size() )
			{
				if( Start.y >= EndLine )
				{
					break;
				}

				Start.y++;
				Start.x = 0;
			}
			else
			{
				Start.x++;
			}
		}
		else
		{
			LineSelectionItem Res( LineSelectionItem::Search );

			Res.Start = Start;
			Res.End   = Next;

			if( WholeWord )
			{
				auto Cmp = []( char C ) -> bool
				{
					return C == '_' || ( C >= 'a' && C <= 'z' ) || ( C >= 'A' && C <= 'Z' ) || ( C >= '0' && C <= '9' );
				};

				const Glyph& rFirstGlyph = pFile->Lines[ Start.y ][ Start.x ];
				const Glyph& rLastGlyph  = pFile->Lines[ Next.y ][ Next.x - 1 ];

				bool Match = true;

				if( Cmp( rFirstGlyph.C ) )
				{
					Coordinate WordStart;
					GetWordAt( *pFile, Start, &WordStart, nullptr );

					Match = WordStart == Start;
				}

				if( Cmp( rLastGlyph.C ) && Match )
				{
					Coordinate WordEnd;
					GetWordAt( *pFile, Coordinate( Next.x - 1, Next.y ), nullptr, &WordEnd );

					Match = WordEnd == Next;
				}

				if( Match )
				{
					pResult->push_back( Res );
				}
			}
			else
			{
				pResult->push_back( Res );
			}

			Start = Next;
		}

		Matches.clear();
	}
} // SearchWorker

//////////////////////////////////////////////////////////////////////////

void TextEdit::SearchManager( File* pFile, bool CaseSensitive, bool WholeWord, SearchInstance* pInstance )
{
	const int    NumThreads = std::thread::hardware_concurrency();
	const double Size       = ( double )( pFile->Lines.size() ) / NumThreads;
	double       Prev       = 0.0;

	std::vector< std::pair< std::thread, std::vector< LineSelectionItem > > > ThreadPool;

	ThreadPool.resize( NumThreads );

	for( int i = 0; i < NumThreads; i++ )
	{
		const int StartLine = static_cast< int >( Prev );
		const int EndLine   = static_cast< int >( Prev + Size ) - 1;
		auto&     rThread   = ThreadPool[ i ];
		rThread.first       = std::thread( &TextEdit::SearchWorker, this, pFile, CaseSensitive, WholeWord, &pInstance->SearchTerm, StartLine, EndLine, &rThread.second, &pInstance->State );

		Prev += Size;
	}

	for( int i = 0; i < ( int )ThreadPool.size(); i++ )
	{
		auto& rThread = ThreadPool[ i ];

		rThread.first.join();

		for( int j = 0; j < ( int )rThread.second.size() && pInstance->State == SearchInstance::Running; j++ )
		{
			pInstance->Result->AddResult( rThread.second[ j ] );
		}
	}

	if( pInstance->State == SearchInstance::Running )
		pInstance->Result->UpdateGroups();

	if( pInstance->State == SearchInstance::Running )
	{
		pInstance->State = SearchInstance::HasResult;
	}
	else
	{
		pInstance->State = SearchInstance::Stopped;
	}
}

void TextEdit::Search( File& rFile, bool CaseSensitive, bool WholeWord, const std::string& rSearchString )
{
	rFile.SearchDiag->ActiveItem = -1;

	for( SearchInstance* pInstance : rFile.SearchDiag->SearchInstances )
	{
		if( pInstance->State == SearchInstance::Running )
		{
			pInstance->State = SearchInstance::Stopping;
		}
	}

	if( rSearchString.empty() ) return;

	SearchInstance* pInstance = new SearchInstance;

	pInstance->SearchTerm = rSearchString;
	pInstance->State      = SearchInstance::Running;
	pInstance->Result     = new SearchResultGroups;
	pInstance->Thread     = std::thread( &TextEdit::SearchManager, this, &rFile, CaseSensitive, WholeWord, pInstance );

	rFile.SearchDiag->SearchInstances.push_back( pInstance );
} // Search

//////////////////////////////////////////////////////////////////////////

void TextEdit::JoinThreads( File& rFile, bool WaitForUnfinished )
{
	std::vector< SearchInstance* >& rInstances = rFile.SearchDiag->SearchInstances;

	if( WaitForUnfinished )
	{
		for( SearchInstance* pInstance : rInstances )
		{
			pInstance->State = SearchInstance::Stopping;
		}

		for( SearchInstance* pInstance : rInstances )
		{
			pInstance->Thread.join();
			delete pInstance;
		}

		rInstances.clear();
		StatusBar::Instance().SetSearchResultInfo( "", 0 );
	}
	else
	{
		for( int i = 0; i < ( int )rInstances.size(); i++ )
		{
			SearchInstance* pInstance = rInstances[ i ];

			if( pInstance->State == SearchInstance::Stopped || pInstance->State == SearchInstance::HasResult )
			{
				pInstance->Thread.join();
				rInstances.erase( rInstances.begin() + i );

				if( pInstance->State == SearchInstance::HasResult )
				{
					delete rFile.SearchDiag->SearchResult;

					rFile.SearchDiag->SearchResult = pInstance->Result;

					StatusBar::Instance().SetSearchResultInfo( pInstance->SearchTerm, ( int )pInstance->Result->Size() );
				}

				delete pInstance;
			}
		}
	}
} // JoinThreads

//////////////////////////////////////////////////////////////////////////

void TextEdit::ShowSearchDialog( File& rFile, ImGuiID FocusId, ImGuiWindow* pWindow )
{
	JoinThreads( rFile, false );

	SearchDialog& rDiag   = *rFile.SearchDiag;
	float         Width   = 350.0f;
	float         Height  = 60.0f;
	float         Padding = 4.0f;
	ImVec2        Cursor  = ImGui::GetCursorPos();
	ImVec2        Size    = ImGui::GetContentRegionAvail();

	Cursor.x = Size.x - Width - 14;

	ImGui::SetCursorPos( Cursor );
	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4( 0xFF080808 ) );
	ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 6.0f );
	ImGui::BeginChild( "##Search", ImVec2( Width, Height ) );
	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 6.0f );

	Cursor.y += Padding;
	Cursor.x += Padding;

	ImGui::SetCursorPos( ImVec2( Padding, Padding ) );

	if( ImGui::IsWindowAppearing() )
	{
		ImGui::SetKeyboardFocusHere( 0 );
	}

	if( ImGui::InputText( "##SearchTerm", &rDiag.SearchTerm ) || Props.Changes )
	{
		Search( rFile, rDiag.CaseSensitive, rDiag.WholeWord, rDiag.SearchTerm );

		if( rDiag.SearchTerm.size() == 0 )
		{
			rDiag.ActiveItem = -1;

			ClearSearch( rFile );
		}
	}

	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( Padding, Padding ) );
	ImGui::SameLine();

	bool Shift = ImGui::GetIO().KeyShift;

	ImGui::BeginDisabled( ( rDiag.SearchResult == nullptr ) || ( rDiag.SearchResult->Empty() ) );

	if( ImGui::Button( "Next" ) || ( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Enter ) ) && ImGui::IsWindowFocused() && !Shift ) )
	{
		int PrevActive = rDiag.ActiveItem;

		rDiag.ActiveItem++;

		if( rDiag.ActiveItem == ( int )rDiag.SearchResult->Size() )
		{
			rDiag.ActiveItem = -1;
		}

		if( PrevActive != -1 )
		{
			LineSelectionItem& rPrev = ( *rDiag.SearchResult )[ PrevActive ];
			rPrev.Type               = LineSelectionItem::Search;
		}

		if( rDiag.ActiveItem != -1 )
		{
			LineSelectionItem& rCurr = ( *rDiag.SearchResult )[ rDiag.ActiveItem ];
			rCurr.Type               = LineSelectionItem::SearchActive;

			ScrollTo( rFile, rCurr.Start, pWindow );
		}
	}

	ImGui::SameLine();

	if( ImGui::Button( "Prev" ) || ( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Enter ) ) && ImGui::IsWindowFocused() && Shift ) )
	{
		int PrevActive = rDiag.ActiveItem;

		if( rDiag.ActiveItem == -1 )
		{
			rDiag.ActiveItem = ( int )rDiag.SearchResult->Size() - 1;
		}
		else
		{
			rDiag.ActiveItem--;
		}

		if( PrevActive != -1 )
		{
			LineSelectionItem& rPrev = ( *rDiag.SearchResult )[ PrevActive ];
			rPrev.Type               = LineSelectionItem::Search;
		}

		if( rDiag.ActiveItem != -1 )
		{
			LineSelectionItem& rCurr = ( *rDiag.SearchResult )[ rDiag.ActiveItem ];
			rCurr.Type               = LineSelectionItem::SearchActive;

			ScrollTo( rFile, rCurr.Start, pWindow );
		}
	}

	ImGui::EndDisabled();

	ImGui::SameLine();

	if( ImGui::Button( "Close" ) || ( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Escape ) ) && ImGui::IsWindowFocused() ) )
	{
		rDiag.Searching  = false;
		rDiag.ActiveItem = -1;
		rDiag.SearchTerm.clear();

		JoinThreads( rFile, true );
		ClearSearch( rFile );

		ImGui::SetFocusID( FocusId, pWindow );
	}

	ImGui::SetCursorPosX( Padding );

	ImGui::TextUnformatted( "Case Sensitive:" );

	ImGui::SameLine();

	if( ImGui::Checkbox( "##CaseSensitivity", &rDiag.CaseSensitive ) )
	{
		if( !rDiag.SearchTerm.empty() )
		{
			Search( rFile, rDiag.CaseSensitive, rDiag.WholeWord, rDiag.SearchTerm );
		}
	}

	ImGui::SameLine();

	ImGui::TextUnformatted( "Whole Word:" );

	ImGui::SameLine();

	if( ImGui::Checkbox( "##WholeWord", &rDiag.WholeWord ) )
	{
		if( !rDiag.SearchTerm.empty() )
		{
			Search( rFile, rDiag.CaseSensitive, rDiag.WholeWord, rDiag.SearchTerm );
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

//////////////////////////////////////////////////////////////////////////

void TextEdit::Undo( File& rFile )
{
	CommandStack& rCommandStack = rFile.CmdStack;

	if( rCommandStack.Location == -1 || rCommandStack.Location >= ( int )rCommandStack.Commands.size() ) return;

	Command* pCmd                  = rCommandStack.Commands[ rCommandStack.Location ];
	rCommandStack.CommandInProcess = true;

	ExecuteCommand( rFile, pCmd );

	rCommandStack.Location--;
	rCommandStack.CommandInProcess = false;

} // Redo

//////////////////////////////////////////////////////////////////////////

void TextEdit::Redo( File& rFile )
{
	CommandStack& rCommandStack = rFile.CmdStack;

	if( ( rCommandStack.Location == -1 && rCommandStack.Commands.empty() ) || rCommandStack.Location + 1 >= ( int )rCommandStack.Commands.size() ) return;

	rCommandStack.Location++;

	Command* pCmd                  = rCommandStack.Commands[ rCommandStack.Location ];
	rCommandStack.CommandInProcess = true;

	ExecuteCommand( rFile, pCmd );

	rCommandStack.CommandInProcess = false;

} // Redo

//////////////////////////////////////////////////////////////////////////

void TextEdit::ExecuteCommand( File& rFile, Command* pCommand )
{
	CommandStack&         rCommandStack = rFile.CmdStack;
	std::vector< Cursor > OldCursors    = rFile.Cursors;

	int NumCommands = ( int )pCommand->SubCommands.size();

	rFile.BoxModeDir      = pCommand->BoxModeDir;
	rFile.CursorMultiMode = pCommand->CursorMultiMode;
	rFile.Cursors         = pCommand->Cursors;

	for( int i = NumCommands - 1; i >= 0; i-- )
	{
		SubCommand* pSub = pCommand->SubCommands[ i ];

		if( pSub->Type == CommandType::Backspace )
		{
			Backspace( rFile, pSub->CursorIndex, true );
		}
		else if( pSub->Type == CommandType::InsertLines )
		{
			Cursor& rCur = pSub->CursorCopy;

			InsertLineMode Mode = InsertLineMode::Default;

			if( rCur.SelectionOrigin != Coordinate( -1, -1 ) )
			{
				if( rCur.SelectionStart == rCur.Position )
				{
					Mode = InsertLineMode::SelectionStart;
				}
				else
				{
					Mode = InsertLineMode::SelectionEnd;
				}
			}
			else if( pSub->NoMove )
			{
				Mode = InsertLineMode::Start;
			}

			InsertLines( rFile, pSub->CursorIndex, pSub->Lines, Mode );

			if( rFile.CursorMultiMode == MultiCursorMode::Box )
				rFile.Cursors[ pSub->CursorIndex ] = pSub->CursorCopy;
		}
		else if( pSub->Type == CommandType::DeleteLines )
		{
			DeleteLines( rFile, pSub->CursorIndex, pSub->Start, pSub->End );
		}
		else if( pSub->Type == CommandType::Enter )
		{
			Enter( rFile, pSub->CursorIndex );

			if( pSub->NoMove )
				rFile.Cursors[ pSub->CursorIndex ] = pSub->CursorCopy;
		}
		else if( pSub->Type == CommandType::EnterText )
		{
			CursorInputMode Prev = rFile.CursorMode;
			rFile.CursorMode     = pCommand->Type == CommandType::Backspace ? CursorInputMode::Normal : pCommand->CursorMode;

			if( pCommand->CursorMode == CursorInputMode::Insert )
				rFile.Cursors[ pSub->CursorIndex ] = pSub->CursorCopy;

			EnterTextStuff( rFile, pSub->Character.C, pSub->CursorIndex, pSub->NoMove );

			rFile.CursorMode = Prev;
		}
		else if( pSub->Type == CommandType::Del )
		{
			Del( rFile, pSub->CursorIndex, true );
		}
		else if( pSub->Type == CommandType::Tab )
		{
			CursorInputMode Prev = rFile.CursorMode;
			rFile.CursorMode     = CursorInputMode::Normal;

			Tab( rFile, pSub->Shift, pSub->CursorIndex, pSub->IgnoreSelection );

			rFile.CursorMode = Prev;
		}
		else if( pSub->Type == CommandType::TabAllCursors )
		{
			Tab( rFile, pSub->Shift );
		}
		else if( pSub->Type == CommandType::RestoreCursor )
		{
			rFile.Cursors[ pSub->CursorIndex ] = pSub->CursorCopy;

			if( pCommand->Type == CommandType::Backspace )
			{
				SubCommand* pTmp = new SubCommand( CommandType::Backspace );

				pTmp->CursorCopy  = pSub->CursorCopy;
				pTmp->CursorIndex = pSub->CursorIndex;

				Command* pTmpCmd = rCommandStack.GetCommand( &rFile, CommandType::Backspace );

				pTmpCmd->AddSubCommand( pTmp );
			}
			else if( pCommand->Type == CommandType::Del || pCommand->Type == CommandType::DeleteLines )
			{
				SubCommand* pTmp = new SubCommand( CommandType::Del );

				pTmp->CursorCopy  = pSub->CursorCopy;
				pTmp->CursorIndex = pSub->CursorIndex;

				Command* pTmpCmd = rCommandStack.GetCommand( &rFile, CommandType::Del );

				pTmpCmd->AddSubCommand( pTmp );
			}
			else if( pCommand->Type == CommandType::None && rFile.CursorMultiMode == MultiCursorMode::Box )
			{
				// This is all kinda meh but it works
				SubCommand* pTmp = new SubCommand( CommandType::RestoreCursor );

				pTmp->CursorCopy  = OldCursors[ pSub->CursorIndex ];
				pTmp->CursorIndex = pSub->CursorIndex;

				Command* pTmpCmd = rCommandStack.GetCommand( &rFile );
				pTmpCmd->Type    = CommandType::None;
				pTmpCmd->AddSubCommand( pTmp );
			}
		}
		else if( pSub->Type == CommandType::Delete )
		{
			DeleteLines( rFile, -1, pSub->Start, pSub->End );
		}
		else if( pSub->Type == CommandType::SwapLines )
		{
			SwapLines( rFile, pSub->Up );
		}
	}

	delete pCommand;

	if( rCommandStack.pCurrentCommand != nullptr ) rCommandStack.FinishCommand( rFile.Cursors );
} // ExecuteCommand

//////////////////////////////////////////////////////////////////////////

// Struct Implementations

SearchResultGroups::SearchResultGroups()
{
	Groups.emplace_back();
}

std::vector< TextEdit::LineSelectionItem* >& SearchResultGroups::GetGroup( int LineIndex )
{
	int Group = LineIndex / SearchResultGroupSize;
	GENO_ASSERT( Group < ( int )Groups.size() );
	return Groups[ Group ];
}

bool SearchResultGroups::GroupExist( int LineIndex )
{
	int Group = LineIndex / SearchResultGroupSize;

	return Group < ( int )Groups.size();
}

void SearchResultGroups::AddResult( const TextEdit::LineSelectionItem& Item )
{
	Result.push_back( Item );
}

void SearchResultGroups::UpdateGroups()
{
	Groups.clear();
	Groups.emplace_back();

	for( TextEdit::LineSelectionItem& rItem : Result )
	{
		const TextEdit::Coordinate& Start = rItem.Start;
		const TextEdit::Coordinate& End   = rItem.End;

		int StartGroup = Start.y / SearchResultGroupSize;
		int EndGroup   = End.y / SearchResultGroupSize;

		for( int i = ( int )Groups.size(); i <= EndGroup; i++ )
		{
			Groups.emplace_back();
		}

		for( int i = StartGroup; i <= EndGroup; i++ )
		{
			Groups[ i ].push_back( &rItem );
		}
	}
}

void SearchResultGroups::Clear()
{
	Groups.clear();
	Result.clear();
	Groups.emplace_back();
}

size_t SearchResultGroups::Size()
{
	return Result.size();
}

bool SearchResultGroups::Empty()
{
	return Size() == 0;
}

TextEdit::LineSelectionItem& SearchResultGroups::operator[]( int Index )
{
	return Result[ Index ];
}

//////////////////////////////////////////////////////////////////////////
