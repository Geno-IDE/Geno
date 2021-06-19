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
#include "GUI/Widgets/MainMenuBar.h"

#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>

const char* WINDOW_NAME = "Text Edit";

float TextEdit::fontSize = 15.0f;

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

	palette.Default             = 0xFFf4f4f4;
	palette.Keyword             = 0xFF0000F0;
	palette.Number              = 0xFF303030;
	palette.String              = 0xFF9E5817;
	palette.Comment             = 0xFF0f5904;
	palette.LineNumber          = 0xFFF0F0F0;
	palette.Selection           = 0x80a06020;
	palette.CurrentLine         = 0x40000000;
	palette.CurrentLineInactive = 0x40808080;
	palette.CurrentLineEdge     = 0x40a0a0a0;
} // TextEdit

//////////////////////////////////////////////////////////////////////////

void TextEdit::Show( bool* pOpen )
{
	ImGuiStyle& rStyle          = ImGui::GetStyle();
	ImVec4      BackgroundColor = rStyle.Colors [ ImGuiCol_WindowBg ];

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
						ImGui::Text( "%ws", rText.c_str() );
						ImGui::EndTooltip();
					}
					break;

					case Drop::TypeIndex::Paths:
					{
						const Drop::Paths& rPaths = pDrop->GetPaths();

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();

						for( const std::filesystem::path& rPath: rPaths )
							ImGui::BulletText( "%ws", rPath.c_str() );

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

		if( !m_Files.empty() && ImGui::BeginTabBarEx( m_pTabBar, TabBarBounds, TabBarFlags, nullptr ) )
		{
			for( File& rFile: m_Files )
			{
				std::string FileString = rFile.Path.filename().string();

				if( rFile.Changed )
				{
					FileString.append( "*" );
				}

				if( ImGui::BeginTabItem( FileString.c_str(), &rFile.Open ) )
				{
					const int InputTextFlags = ImGuiInputTextFlags_AllowTabInput;

					ImGui::PushFont( MainWindow::Instance().GetFontMono() );

					if( RenderEditor( rFile ) )
					{
						rFile.Changed = true;
					}

					ImGui::PopFont();
					ImGui::EndTabItem();
				}
			}

			// Clear closed files from list
			for( auto It = m_Files.begin(); It != m_Files.end(); )
			{
				if( It->Open ) It++;
				else
					It = m_Files.erase( It );
			}

			ImGui::EndTabBar();
		}
	}
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

	for( int i = 0; i < m_Files.size(); ++i )
	{
		File& rFile = m_Files [ i ];

		// Do not need to add file to vector if it already exists
		if( rFile.Path == rPath )
		{
			// Select the tab that corresponds to the open file
			m_pTabBar->NextSelectedTabId = m_pTabBar->Tabs [ static_cast< int >( i ) ].ID;

			// Update text in case file changed externally
			rFile.Text = Text;

			SplitLines( rFile );

			return;
		}
	}

	File File;
	File.Path = rPath;
	File.Text = Text;

	SplitLines( File );

	m_Files.emplace_back( std::move( File ) );

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

				for( const std::filesystem::path& rPath: rPaths )
					AddFile( rPath );
			}
			break;
		}
	}

} // OnDragDrop

void TextEdit::SplitLines( File& file )
{
	file.Lines.clear();

	Line lineBuffer;

	for( int i = 0; i < file.Text.length(); i++ )
	{
		const char c = file.Text [ i ];

		if( c == '\n' )
		{
			file.Lines.push_back( lineBuffer );
			lineBuffer.clear();
		}
		else
		{
			lineBuffer.push_back( Glyph( c, palette.Default ) );
		}
	}

	file.Lines.push_back( lineBuffer );
}

bool TextEdit::RenderEditor( File& file )
{
	props.Changes = false;
	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4( 0xFF101010 ) );
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

	ImGui::PushAllowKeyboardFocus( true );

	HandleKeyboardInputs( file );

	CalculeteLineNumMaxWidth( file );

	props.CharAdvanceY = ImGui::GetTextLineHeightWithSpacing();

	ImVec2 size   = ImGui::GetContentRegionMax();
	ImVec2 cursor = ImGui::GetCursorScreenPos();

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::SetCursorScreenPos( ImVec2( cursor.x + props.LineNumMaxWidth, cursor.y ) );
	ImGui::BeginChild( "##TextEditor", ImVec2( size.x - props.LineNumMaxWidth, 0 ), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar );

	HandleMouseInputs( file );

	props.ScrollX = ImGui::GetScrollX();
	props.ScrollY = ImGui::GetScrollY();

	int firstLine = ( int )( props.ScrollY / props.CharAdvanceY );
	int lastLine  = std::min( firstLine + ( int )( size.y / props.CharAdvanceY + 2 ), ( int )file.Lines.size() - 1 );

	float longest = 0.0f;

	for( int i = firstLine; i <= lastLine; i++ )
	{
		ImVec2 pos( cursor.x + props.LineNumMaxWidth - props.ScrollX, cursor.y + ( i - firstLine ) * props.CharAdvanceY );
		Line&  line = file.Lines [ i ];

		Coordinate selectedStart;
		Coordinate selectedEnd;

		if( IsLineSelected( file, i, &selectedStart, &selectedEnd ) )
		{
			ImVec2 start( cursor.x + props.LineNumMaxWidth + GetDistance( file, selectedStart ), pos.y );
			ImVec2 end( cursor.x + props.LineNumMaxWidth + GetDistance( file, selectedEnd ), pos.y + props.CharAdvanceY );

			drawList->AddRectFilled( start, end, palette.Selection );
		}

		for( int j = 0; j < file.cursors.size(); j++ )
		{
			Cursor c = file.cursors [ j ];

			if( c.disabled ) continue;

			if( c.position.y == i )
			{
				bool focus = ImGui::IsWindowFocused();

				if( !HasSelection( file, j ) )
				{
					ImVec2 start( cursor.x + props.LineNumMaxWidth, pos.y );
					ImVec2 end( cursor.x + size.x, pos.y + props.CharAdvanceY );

					drawList->AddRectFilled( start, end, focus ? palette.CurrentLine : palette.CurrentLineInactive );
					drawList->AddRect( start, end, palette.CurrentLineEdge );
				}

				if( focus )
				{
					static auto start   = std::chrono::system_clock::now();
					auto        now     = std::chrono::system_clock::now();
					long long   elapsed = std::chrono::duration_cast< std::chrono::milliseconds >( now - start ).count();

					if( elapsed >= props.CursorBlink )
					{
						elapsed -= props.CursorBlink;

						float  cursorPos = GetCursorDistance( file, j );
						ImVec2 cStart( pos.x + cursorPos, pos.y );
						ImVec2 cEnd( cStart.x + 1.0f, cStart.y + props.CharAdvanceY - 1 );

						drawList->AddRectFilled( cStart, cEnd, palette.Cursor );

						if( elapsed >= props.CursorBlink )
							start = now;
					}
				}
			}
		}

		std::string stringBuffer;

		float        xOffset   = 0.0f;
		unsigned int prevColor = ( unsigned int )line.size() ? line [ 0 ].color : palette.Default;

		for( Glyph& glyph: line )
		{
			if( glyph.color != prevColor )
			{
				drawList->AddText( ImVec2( pos.x + xOffset, pos.y ), prevColor, stringBuffer.c_str() );
				float textWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, stringBuffer.c_str() ).x;
				xOffset += textWidth;
				stringBuffer.clear();

				prevColor = glyph.color;
			}
			else
			{
				stringBuffer.push_back( glyph.c );
			}
		}

		if( !stringBuffer.empty() )
		{
			drawList->AddText( ImVec2( pos.x + xOffset, pos.y ), prevColor, stringBuffer.c_str() );
			float textWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, stringBuffer.c_str() ).x;
			xOffset += textWidth;
			stringBuffer.clear();
		}

		if( xOffset > longest ) longest = xOffset;
	}

	ImGui::Dummy( ImVec2( longest + 10, ( file.Lines.size() + 10 ) * props.CharAdvanceY ) );

	ImGui::PopAllowKeyboardFocus();
	ImGui::EndChild();

	//Render line numbers
	ImGui::SetCursorScreenPos( ImVec2( cursor.x - 2, cursor.y ) );
	ImGui::BeginChild( "##LineNumbers", ImVec2( props.LineNumMaxWidth, size.y + 2 ), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

	char buf [ 32 ];

	for( int i = firstLine; i <= lastLine; i++ )
	{
		sprintf( buf, "%u | ", i + 1 );

		const float currentLineNumWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, buf ).x;
		ImVec2      pos( cursor.x + props.LineNumMaxWidth - currentLineNumWidth, cursor.y + ( i - firstLine ) * props.CharAdvanceY );

		drawList->AddText( pos, palette.LineNumber, buf );
	}

	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	return props.Changes;
}

void TextEdit::HandleKeyboardInputs( File& file )
{
	ImGuiIO& io = ImGui::GetIO();

	bool shift = io.KeyShift;
	bool ctrl  = io.KeyCtrl;
	bool alt   = io.KeyAlt;

	// Keyboard Inputs
	io.WantCaptureKeyboard = true;
	io.WantTextInput       = true;

	if( !shift && !ctrl & !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Enter ) ) )
		EnterTextStuff( file, ImGuiKey_Enter );
	else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Backspace ) ) )
		EnterTextStuff( file, ImGuiKey_Backspace );
	else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow ) ) )
		MoveUp( file );
	else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_DownArrow ) ) )
		MoveDown( file );
	else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow ) ) )
		MoveRight( file );
	else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow ) ) )
		MoveLeft( file );

	for( int i = 0; i < io.InputQueueCharacters.Size; i++ )
	{
		char c = ( char )io.InputQueueCharacters [ i ];

		EnterTextStuff( file, c );
	}
}

void TextEdit::HandleMouseInputs( File& file )
{
	ImGuiIO& io = ImGui::GetIO();

	bool shift = io.KeyShift;
	bool ctrl  = io.KeyCtrl;
	bool alt   = io.KeyAlt;

	if( ImGui::IsWindowHovered() )
	{
		// Mouse Inputs
		ImGui::SetMouseCursor( ImGuiMouseCursor_TextInput );

		bool clicked       = ImGui::IsMouseClicked( ImGuiMouseButton_Left );
		bool doubleClicked = ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left );
		bool dragged       = ImGui::IsMouseDragging( ImGuiMouseButton_Left );

		if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) || props.Changes )
		{
			file.cursors [ file.cursors.size() - 1 ].selectionOrigin = props.Changes ? GetMouseCoordinate( file ) : Coordinate( ~0, ~0 );

			for( int i = 0; i < file.cursors.size(); i++ )
			{
				if( file.cursors [ i ].disabled ) file.cursors.erase( file.cursors.begin() + i-- );
			}
		}

		if( doubleClicked )
		{
			Cursor& lastCursor = file.cursors [ file.cursors.size() - 1 ];

			std::string word = GetWordAt( file, lastCursor );

			if( word.empty() ) return;

			lastCursor.position = lastCursor.selectionEnd;
		}
		else if( clicked )
		{
			Cursor c;
			c.position = c.selectionOrigin = GetMouseCoordinate( file );

			if( ctrl && !( alt || shift ) )
			{
			}
			else if( shift && !( alt || ctrl ) )
			{
				file.cursors.erase( file.cursors.begin() + 1, file.cursors.end() );

				Cursor& cur = file.cursors [ 0 ];

				cur.selectionStart = cur.selectionOrigin = cur.position;
				cur.selectionEnd = cur.position = c.position;
			}
			else
			{
				if( !( ctrl && alt ) ) file.cursors.clear();

				if( !IsCoordinateInSelection( file, c.position ) )
					file.cursors.push_back( c );
			}
		}
		else if( dragged )
		{
			Coordinate pos    = GetMouseCoordinate( file );
			Cursor&    cursor = file.cursors [ file.cursors.size() - 1 ];

			if( cursor.selectionOrigin != Coordinate( ~0, ~0 ) )
			{
				if( pos > cursor.selectionOrigin )
				{
					cursor.selectionEnd   = pos;
					cursor.selectionStart = cursor.selectionOrigin;
				}
				else
				{
					cursor.selectionStart = pos;
					cursor.selectionEnd   = cursor.selectionOrigin;
				}

				cursor.position = pos;

				DisableIntersectingSelections( file, ( int )file.cursors.size() - 1 );
			}
		}

		if( io.MouseWheel != 0 && io.KeyCtrl && !io.KeyAlt && !io.KeyShift )
			fontSize += io.MouseWheel;
	}
}

void TextEdit::CalculeteLineNumMaxWidth( File& file )
{
	int totalLines = ( int )file.Lines.size();

	char buf [ 32 ];
	sprintf( buf, " %u | ", totalLines );

	props.LineNumMaxWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, buf ).x;
}

bool TextEdit::HasSelection( File& file, int cursor ) const
{
	const Cursor& c = file.cursors [ cursor ];

	if( c.disabled ) return false;

	if( c.selectionStart.y != c.selectionEnd.y )
		return c.selectionEnd.y > c.selectionStart.y;

	return c.selectionEnd.x > c.selectionStart.x;
}

bool TextEdit::IsCoordinateInSelection( File& file, Coordinate coordinate )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( ( coordinate > c.selectionStart && coordinate < c.selectionEnd ) || ( coordinate == c.position ) )
			return true;
	}

	return false;
}

bool TextEdit::IsLineSelected( File& file, int line, Coordinate* start, Coordinate* end ) const
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		const Cursor& c = file.cursors [ i ];

		if( c.selectionStart == c.selectionEnd || c.disabled ) continue;

		if( line == c.selectionStart.y )
		{
			*start = c.selectionStart;

			if( line == c.selectionEnd.y )
			{
				*end = c.selectionEnd;
			}
			else
			{
				end->y = line;
				end->x = ( int )file.Lines [ line ].size();
			}

			return true;
		}
		else if( line >= c.selectionStart.y && line <= c.selectionEnd.y )
		{
			start->x = 0;
			start->y = line;

			if( line == c.selectionEnd.y )
			{
				*end = c.selectionEnd;
			}
			else
			{
				end->y = line;
				end->x = ( int )file.Lines [ line ].size();
			}

			return true;
		}
	}

	return false;
}

float TextEdit::GetCursorDistance( File& file, int cursor ) const
{
	return GetDistance( file, file.cursors [ cursor ].position );
}

float TextEdit::GetDistance( File& file, Coordinate position ) const
{
	const Line& line = file.Lines [ position.y ];

	char* string = new char [ position.x + 1 ];

	string [ position.x ] = 0;

	for( int i = 0; i < position.x; i++ )
	{
		string [ i ] = line [ i ].c;
	}

	float res = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, string ).x;

	delete [] string;

	return res;
}

std::string TextEdit::GetWordAt( File& file, Cursor& cursor ) const
{
	return GetWordAt( file, cursor.position, &cursor.selectionStart, &cursor.selectionEnd );
}

std::string TextEdit::GetWordAt( File& file, Coordinate position, Coordinate* start, Coordinate* end ) const
{
	const Line& l = file.Lines [ position.y ];

	if( position.x >= ( int )l.size() )
	{
		if( position.x == 0 ) return std::string();
		position.x -= 1;
	}

	char c = l [ position.x ].c;

	std::string buffer;

	auto getRegion = [ &buffer, &l, &position, start, end ]( bool ( *cmpFunc )( char c ) ) -> std::string {
		int len = ( int )l.size();
		int x0  = 0;
		int x1  = len;

		for( int i = position.x + 1; i < len; i++ )
		{
			char chr = l [ i ].c;
			if( !cmpFunc( chr ) )
			{
				x1 = i;
				break;
			}

			buffer.push_back( chr );
		}

		for( int i = position.x; i > 0; i-- )
		{
			char chr = l [ i ].c;
			if( !cmpFunc( chr ) )
			{
				x0 = i + 1;
				break;
			}

			buffer.insert( buffer.begin(), chr );
		}

		if( start ) *start = Coordinate( x0, position.y );
		if( end ) *end = Coordinate( x1, position.y );

		return std::move( buffer );
	};

	auto cmpCharsNum = []( char c ) -> bool {
		return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' );
	};

	auto cmpWhitespace = []( char c ) -> bool {
		return c == ' ' || c == '\t';
	};

	auto cmpOperators = []( char c ) -> bool {
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '<' || c == '>' || c == '|' || c == '&' || c == '^' || c == '!' || c == '=';
	};

	if( cmpCharsNum( c ) )
	{
		return getRegion( cmpCharsNum );
	}
	else if( cmpWhitespace( c ) )
	{
		char lc = position.x == 0 ? ' ' : l [ position.x - 1 ].c;

		if( cmpWhitespace( lc ) )
		{
			return getRegion( cmpWhitespace );
		}
		else if( cmpOperators( lc ) )
		{
			position.x--;
			return getRegion( cmpOperators );
		}
		else if( cmpCharsNum( lc ) )
		{
			position.x--;
			return getRegion( cmpCharsNum );
		}
		else
		{
			if( start ) *start = Coordinate( position.x - 1, position.y );
			if( end ) *end = Coordinate( position.x, position.y );
		}
	}
	else if( cmpOperators( c ) )
	{
		return getRegion( cmpOperators );
	}
	else
	{
		if( start ) *start = Coordinate( position.x, position.y );
		if( end ) *end = Coordinate( position.x + 1, position.y );
	}

	return std::string();
}

void TextEdit::SetSelectionLine( File& file, int line )
{
	if( line >= file.Lines.size() ) return;

	Line& l = file.Lines [ line ];

	Coordinate start( 0, line );
	Coordinate end( ( int )l.size(), line );

	SetSelection( file, start, end, 0 );
}

void TextEdit::SetSelection( File& file, Coordinate start, Coordinate end, int cursor )
{
	GENO_ASSERT( start.x < end.x && start.y < end.y );
	GENO_ASSERT( cursor < file.cursors.size() );

	auto& lines = file.Lines;

	if( start.y >= lines.size() ) return;

	if( end.y >= lines.size() )
	{
		end.y = ( int )lines.size() - 1;

		end.x = ( int )lines [ end.y ].size();
	}

	Cursor& c = file.cursors [ cursor ];

	c.selectionStart = start;
	c.selectionEnd   = end;
}

TextEdit::Coordinate TextEdit::GetMouseCoordinate( File& file, float* distance )
{
	ImVec2 origin   = ImGui::GetCursorScreenPos();
	ImVec2 mousePos = ImGui::GetMousePos();

	mousePos.x -= origin.x;
	mousePos.y -= origin.y;

	int line = ( int )( ( ( mousePos.y + props.ScrollY ) / props.CharAdvanceY ) - ( props.ScrollY / props.CharAdvanceY ) );

	int numLines = ( int )file.Lines.size();

	if( line > numLines - 1 )
	{
		line = numLines - 1;
	}

	const Line& l = file.Lines [ line ];

	int lineSize = ( int )l.size();

	char* string = new char [ lineSize + 1 ];
	memset( string, 0, lineSize + 1 );

	float length = 0.0f;

	for( int i = 0; i < l.size(); i++ )
	{
		string [ i ] = l [ i ].c;

		float currLength = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, string ).x;
		float diff       = currLength - length;

		if( currLength - ( diff / 2.0f ) > mousePos.x )
		{
			delete [] string;
			if( distance ) *distance = length;
			return Coordinate( i, line );
		}

		length = currLength;
	}

	delete [] string;

	if( distance ) *distance = length;

	return Coordinate( ( int )l.size(), line );
}

void TextEdit::AdjustCursors( File& file, int cursor, int xOffset, int yOffset )
{
	Cursor& c = file.cursors [ cursor ];
	for( int j = 0; j < file.cursors.size(); j++ )
	{
		if( j == cursor ) continue;

		Cursor& other = file.cursors [ j ];

		if( other.selectionStart > c.selectionStart )
		{
			if( other.selectionStart.y == c.selectionStart.y )
			{
				other.selectionStart.x -= c.selectionStart.x;

				if( other.selectionEnd.y == c.selectionStart.y )
				{
					other.selectionStart.x -= c.selectionStart.x;
				}
			}
			else
			{
				other.selectionStart.y -= yOffset;
				other.selectionEnd.y -= yOffset;
			}
		}

		if( other.position > c.position )
		{
			if( other.position.y == c.position.y )
			{
				other.position.x -= xOffset;
			}

			other.position.y -= yOffset;
		}
	}
}

void TextEdit::YeetDuplicateCursors( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];
		for( int j = i + 1; j < file.cursors.size(); j++ )
		{
			Cursor& tmpC = file.cursors [ j ];

			if( tmpC.position == c.position )
			{
				file.cursors.erase( file.cursors.begin() + j );
			}
		}
	}
}

void TextEdit::DisableIntersectingSelections( File& file, int cursor )
{
	Cursor& c = file.cursors [ cursor ];

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		if( i == cursor ) continue;

		Cursor& c2 = file.cursors [ i ];

		if( ( c2.selectionStart > c.selectionStart && c2.selectionStart < c.selectionEnd ) || ( c2.selectionEnd > c.selectionStart && c2.selectionEnd < c.selectionEnd ) )
		{
			c2.disabled = true;
		}
		else
		{
			c2.disabled = false;
		}
	}
}

void TextEdit::Enter( File& file, int cursor )
{
	Cursor& c = file.cursors [ cursor ];

	auto& lines = file.Lines;

	Line  newLine;
	Line& line = lines [ c.position.y ];

	bool ass = c.position.x != line.size();

	if( ass )
	{
		auto start = line.begin() + c.position.x;
		newLine.insert( newLine.begin(), start, line.end() );
		line.erase( start, line.end() );
	}

	AdjustCursors( file, cursor, c.position.x, ass ? -1 : 0 );

	c.position.y++;
	c.position.x = 0;
	lines.insert( lines.begin() + c.position.y, newLine );
}

void TextEdit::Backspace( File& file, int cursor )
{
	Cursor& c = file.cursors [ cursor ];

	auto& lines = file.Lines;

	if( HasSelection( file, cursor ) )
	{
		Line& l = lines [ c.selectionStart.y ];

		int yOffset = 0;
		int xOffset = 0;

		if( c.selectionEnd.y == c.selectionStart.y )
		{
			l.erase( l.begin() + c.selectionStart.x, l.begin() + c.selectionEnd.x );

			xOffset = c.selectionEnd.x - c.selectionStart.x;
		}
		else
		{
			l.erase( l.begin() + c.selectionStart.x, l.end() );

			int numLines = ( int )c.selectionEnd.y - c.selectionStart.y - 1;

			if( numLines > 0 )
			{
				lines.erase( lines.begin() + c.selectionStart.y + 1, lines.begin() + c.selectionStart.y + numLines + 1 );
			}

			Line& l2 = lines [ c.selectionStart.y + 1 ];

			yOffset = numLines + 1;
			xOffset = c.selectionEnd.x - 1;

			if( xOffset == ~0 ) xOffset++;

			l2.erase( l2.begin(), l2.begin() + xOffset );

			if( !l2.empty() )
			{
				l.insert( l.begin(), l2.begin(), l2.end() );
			}

			lines.erase( lines.begin() + c.selectionStart.y + 1 );
		}

		AdjustCursors( file, cursor, xOffset, yOffset );

		c.position = c.selectionStart;

		c.selectionStart = { 0, 0 };
		c.selectionEnd   = { 0, 0 };
	}
	else
	{
		Line& line = lines [ c.position.y ];

		if( c.position.x == 0 && c.position.y != 0 )
		{
			Line& lineAbove = lines [ c.position.y - 1 ];

			int x = ( int )lineAbove.size();

			if( !line.empty() )
			{
				lineAbove.insert( lineAbove.end(), line.begin(), line.end() );
			}

			lines.erase( lines.begin() + c.position.y );

			AdjustCursors( file, cursor, -x, 1 );

			c.position.x = x;
			c.position.y--;
		}
		else if( !( c.position.y == 0 && c.position.x == 0 ) )
		{
			c.position.x--;
			line.erase( line.begin() + c.position.x );

			AdjustCursors( file, cursor, 1, 0 );
		}

		YeetDuplicateCursors( file );
	}
}

void TextEdit::EnterTextStuff( File& file, char c )
{
	props.Changes = true;

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& cursor = file.cursors [ i ];

		if( HasSelection( file, i ) )
		{
			Backspace( file, i );
		}
		else if( c == ImGuiKey_Backspace )
		{
			Backspace( file, i );
			continue;
		}

		if( c == ImGuiKey_Enter )
		{
			Enter( file, i );
			continue;
		}

		Line& l = file.Lines [ cursor.position.y ];

		l.insert( l.begin() + cursor.position.x, Glyph( c, palette.Default ) );

		cursor.position.x++;

		AdjustCursors( file, i, -1, 0 );
	}
}

void TextEdit::MoveUp( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.position.y == 0 ) continue;

		c.position.y--;

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x > ( int )line.size() ) c.position.x = ( int )line.size();

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors( file );
}

void TextEdit::MoveDown( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.position.y == ( int )file.Lines.size() - 1 ) continue;

		c.position.y++;

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x > ( int )line.size() ) c.position.x = ( int )line.size();

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors( file );
}

void TextEdit::MoveRight( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x == ( int )line.size() && c.position.y != ( int )file.Lines.size() )
		{
			c.position.x = 0;
			c.position.y++;
		}
		else
		{
			c.position.x++;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors( file );
}

void TextEdit::MoveLeft( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.position.x == 0 && c.position.y != 0 )
		{
			Line& line   = file.Lines [ --c.position.y ];
			c.position.x = ( int )line.size();
		}
		else
		{
			c.position.x--;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors( file );
}
