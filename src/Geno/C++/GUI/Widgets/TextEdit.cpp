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

constexpr float TabSize = 4.0f;

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
	palette.Cursor              = 0xFFf8f8f8;
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

				if( ImGui::BeginTabItem( FileString.c_str(), &rFile.Open, rFile.Changed ? ImGuiTabItemFlags_UnsavedDocument : 0 ) )
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

	CalculeteLineNumMaxWidth( file );

	props.CharAdvanceY = ImGui::GetTextLineHeightWithSpacing();
	props.SpaceSize    = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, " " ).x;

	ImVec2 size   = ImGui::GetContentRegionMax();
	ImVec2 cursor = ImGui::GetCursorScreenPos();

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::SetCursorScreenPos( ImVec2( cursor.x + props.LineNumMaxWidth, cursor.y ) );
	ImGui::BeginChild( "##TextEditor", ImVec2( size.x - props.LineNumMaxWidth, 0 ), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar );
	HandleKeyboardInputs( file );
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

		Coordinate selectedStart [ 16 ];
		Coordinate selectedEnd [ 16 ];

		if( int count = IsLineSelected( file, i, selectedStart, selectedEnd ) )
		{
			for( int j = 0; j < count; j++ )
			{
				ImVec2 start( cursor.x + props.LineNumMaxWidth + GetDistance( file, selectedStart [ j ] ) - props.ScrollX, pos.y );
				ImVec2 end( cursor.x + props.LineNumMaxWidth + GetDistance( file, selectedEnd [ j ] ) - props.ScrollX, pos.y + props.CharAdvanceY );

				drawList->AddRectFilled( start, end, palette.Selection );
			}
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
			if( glyph.color != prevColor || glyph.c == '\t' )
			{
				drawList->AddText( ImVec2( pos.x + xOffset, pos.y ), prevColor, stringBuffer.c_str() );
				float textWidth = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, stringBuffer.c_str() ).x;
				xOffset += textWidth;
				stringBuffer.clear();

				prevColor = glyph.color;

				if( glyph.c == '\t' )
				{
					float tab = TabSize * props.SpaceSize;

					xOffset += tab;

					float fraction = xOffset / tab;
					fraction       = fraction - floorf( fraction );

					xOffset -= tab * fraction;
				}
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
	if( ImGui::IsWindowFocused() )
	{
		ImGuiIO& io = ImGui::GetIO();

		bool shift = io.KeyShift;
		bool ctrl  = io.KeyCtrl;
		bool alt   = io.KeyAlt;

		// Keyboard Inputs
		io.WantCaptureKeyboard = true;
		io.WantTextInput       = true;

		if( !shift && !ctrl & !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Enter ) ) )
			Enter( file );
		else if( !shift && !ctrl && !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Backspace ) ) )
			Backspace( file );
		else if( !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow ) ) )
			MoveUp( file, shift );
		else if( !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_DownArrow ) ) )
			MoveDown( file, shift );
		else if( !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow ) ) )
			MoveRight( file, ctrl, shift );
		else if( !alt && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow ) ) )
			MoveLeft( file, ctrl, shift );
		else if( !alt && !ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Delete ) ) )
			Del( file );
		else if( !alt && !ctrl && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Tab ) ) )
			Tab( file, shift );
		else if( !alt && !ctrl && !shift && ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Escape ) ) )
			file.cursors.erase( file.cursors.begin() + 1, file.cursors.end() );

		for( int i = 0; i < io.InputQueueCharacters.Size; i++ )
		{
			char c = ( char )io.InputQueueCharacters [ i ];

			EnterTextStuff( file, c );
		}
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

		if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) || ( props.Changes && ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) )
		{
			if( props.Changes )
				file.cursors [ file.cursors.size() - 1 ].selectionOrigin = GetCoordinate( file, ImGui::GetMousePos() );

			DeleteDisabledCursor( file );
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
			c.position = c.selectionOrigin = GetCoordinate( file, ImGui::GetMousePos() );

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

				if( IsCoordinateInSelection( file, c.position ) == nullptr )
					file.cursors.push_back( c );
			}
		}
		else if( dragged )
		{
			Coordinate pos    = GetCoordinate( file, ImGui::GetMousePos() );
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

void TextEdit::ScrollToCursor( File& file )
{
	float yScroll = ImGui::GetScrollY();
	float xScroll = ImGui::GetScrollX();

	ImVec2 size = ImGui::GetContentRegionAvail();

	float top    = yScroll;
	float bottom = top + size.y - ( props.CharAdvanceY * 2 );
	float left   = xScroll;
	float right  = left + size.x - 10.0f;

	Cursor& cursor = file.cursors [ 0 ];

	float cx = GetDistance( file, cursor.position );
	float cy = cursor.position.y * props.CharAdvanceY;

	if( cy < top )
	{
		ImGui::SetScrollY( cursor.position.y * props.CharAdvanceY );
	}
	else if( cy > bottom )
	{
		ImGui::SetScrollY( cursor.position.y * props.CharAdvanceY - size.y + ( props.CharAdvanceY * 2 ) );
	}

	if( cx < left )
	{
		ImGui::SetScrollX( cx );
	}
	else if( cx > right )
	{
		ImGui::SetScrollX( cx - size.x + 10.0f );
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

TextEdit::Cursor* TextEdit::IsCoordinateInSelection( File& file, Coordinate coordinate, int offset )
{
	GENO_ASSERT( offset <= file.cursors.size() );

	for( int i = offset; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.disabled ) continue;

		if( ( coordinate > c.selectionStart && coordinate < c.selectionEnd ) || ( coordinate == c.position ) )
		{
			return &c;
		}
	}

	return nullptr;
}

int TextEdit::IsLineSelected( File& file, int line, Coordinate* start, Coordinate* end ) const
{
	int count = 0;

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		const Cursor& c = file.cursors [ i ];

		if( c.selectionStart == c.selectionEnd || c.disabled ) continue;

		if( line == c.selectionStart.y )
		{
			*( start + count ) = c.selectionStart;

			if( line == c.selectionEnd.y )
			{
				*( end + count ) = c.selectionEnd;
			}
			else
			{
				( end + count )->y = line;
				( end + count )->x = ( int )file.Lines [ line ].size();
			}

			count++;
		}
		else if( line >= c.selectionStart.y && line <= c.selectionEnd.y )
		{
			( start + count )->x = 0;
			( start + count )->y = line;

			if( line == c.selectionEnd.y )
			{
				*( end + count ) = c.selectionEnd;
			}
			else
			{
				( end + count )->y = line;
				( end + count )->x = ( int )file.Lines [ line ].size();
			}

			count++;
		}
	}

	return count;
}

float TextEdit::GetCursorDistance( File& file, int cursor ) const
{
	return GetDistance( file, file.cursors [ cursor ].position );
}

float TextEdit::GetDistance( File& file, Coordinate position ) const
{
	const Line& line = file.Lines [ position.y ];

	std::string string;

	float xOffset = 0.0f;

	position.x = position.x >= ( int )line.size() ? ( int )line.size() - 1 : position.x;

	for( int i = 0; i < position.x; i++ )
	{
		char c = line [ i ].c;

		if( c == '\t' )
		{
			xOffset += ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, string.c_str() ).x;

			float tab = TabSize * props.SpaceSize;

			xOffset += tab;

			float fraction = xOffset / tab;
			fraction       = fraction - floorf( fraction );

			xOffset -= fraction * tab;
			string.clear();
		}
		else
		{
			string.push_back( c );
		}
	}

	if( !string.empty() )
	{
		xOffset += ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, string.c_str() ).x;
	}

	return xOffset;
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

		for( int i = position.x; i >= 0; i-- )
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

		std::string res;
		res.push_back( c );

		return std::move( res );
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

TextEdit::Coordinate TextEdit::GetCoordinate( File& file, ImVec2 position, bool relativeToEditor )
{
	if( !relativeToEditor )
	{
		ImVec2 origin = ImGui::GetCursorScreenPos();

		position.x -= origin.x;
		position.y -= origin.y;
	}

	int line = ( int )( ( ( position.y + props.ScrollY ) / props.CharAdvanceY ) - ( props.ScrollY / props.CharAdvanceY ) );

	int numLines = ( int )file.Lines.size();

	if( line > numLines - 1 )
	{
		line = numLines - 1;
	}

	const Line& l = file.Lines [ line ];

	int lineSize = ( int )l.size();

	char string[2] = { 0, 0 };

	float length = 0.0f;

	for( int i = 0; i < l.size(); i++ )
	{
		/*string[i] = l[i].c;

		float currLength = ImGui::GetFont()->CalcTextSizeA( ImGui::GetFontSize(), FLT_MAX, -1.0f, string ).x;
		float diff       = currLength - length;

		if( currLength - ( diff / 2.0f ) > position.x )
		{
			delete [] string;
			return Coordinate( i, line );
		}

		length = currLength;*/

		string[0] = l[i].c;

		float diff = 0.0f;

		if (string[0] == '\t') {
			diff = TabSize * props.SpaceSize;
			length = CalculateTabAlignmentDistance(file, Coordinate(i+1, line)) + diff;
		} else {
			length += diff = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, string).x;
		}

		if (length - (diff / 2.0f) > position.x) {
			return Coordinate(i, line);
		}

	}

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
				other.selectionStart.x -= xOffset;

				if( other.selectionEnd.y == c.selectionStart.y )
				{
					other.selectionEnd.x -= xOffset;
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

void TextEdit::DeleteDisabledCursor( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		if( file.cursors [ i ].disabled ) file.cursors.erase( file.cursors.begin() + i-- );
	}
}

void TextEdit::DeleteSelection( File& file, int cursor )
{
	Cursor& c = file.cursors [ cursor ];

	if( c.disabled ) return;

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
			xOffset = c.selectionEnd.x;

			if( xOffset == ~0 ) xOffset++;

			l2.erase( l2.begin(), l2.begin() + xOffset );

			if( !l2.empty() )
			{
				l.insert( l.end(), l2.begin(), l2.end() );
			}

			lines.erase( lines.begin() + c.selectionStart.y + 1 );
		}

		AdjustCursors( file, cursor, xOffset, yOffset );

		c.position = c.selectionStart;

		c.selectionStart  = { 0, 0 };
		c.selectionEnd    = { 0, 0 };
		c.selectionOrigin = { -1, -1 };
	}
}

void TextEdit::Enter( File& file )
{
	props.Changes = true;

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.disabled ) continue;

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

		AdjustCursors( file, i, c.position.x, ass ? -1 : 0 );

		c.position.y++;
		c.position.x = 0;
		lines.insert( lines.begin() + c.position.y, newLine );
	}

	ScrollToCursor( file );
}

void TextEdit::Backspace( File& file )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		if( HasSelection( file, i ) )
		{
			DeleteSelection( file, i );
		}
		else
		{
			Cursor& c = file.cursors [ i ];

			if( c.disabled ) continue;

			auto& lines = file.Lines;

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

				AdjustCursors( file, i, -x, 1 );

				c.position.x = x;
				c.position.y--;
			}
			else if( !( c.position.y == 0 && c.position.x == 0 ) )
			{
				c.position.x--;
				line.erase( line.begin() + c.position.x );

				AdjustCursors( file, i, 1, 0 );
			}

			YeetDuplicateCursors( file );
		}
	}

	ScrollToCursor( file );
}

void TextEdit::Del( File& file )
{
	props.Changes = true;

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		if( HasSelection( file, i ) )
		{
			DeleteSelection( file, i );
		}
		else
		{
			Cursor& c = file.cursors [ i ];

			if( c.disabled ) continue;

			Line& l = file.Lines [ c.position.y ];

			l.erase( l.begin() + c.position.x );
		}
	}

	ScrollToCursor( file );
}

void TextEdit::Tab( File& file, bool shift )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.disabled ) continue;

		if( shift )
		{
		}
		else
		{
			if( HasSelection( file, i ) )
			{
				if( c.selectionStart.y == c.selectionEnd.y )
				{
					DeleteSelection( file, i );
				}
				else
				{
					for( int j = c.selectionStart.y; j <= c.selectionEnd.y; j++ )
					{
						Line& l = file.Lines [ j ];

						l.insert( l.begin(), Glyph( '\t', palette.Default ) );
					}

					c.position.x++;
					c.selectionStart.x++;
					c.selectionEnd.x++;
					c.selectionOrigin.x++;

					continue;
				}
			}

			Line& l = file.Lines [ c.position.y ];

			l.insert( l.begin() + c.position.x, Glyph( '\t', palette.Default ) );

			c.position.x++;

			AdjustCursors( file, i, -1, 0 );
		}
	}

	ScrollToCursor( file );
}

void TextEdit::EnterTextStuff( File& file, char c, bool shift )
{
	props.Changes = true;

	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& cursor = file.cursors [ i ];

		if( cursor.disabled ) continue;

		if( HasSelection( file, i ) )
		{
			DeleteSelection( file, i );
		}

		Line& l = file.Lines [ cursor.position.y ];

		l.insert( l.begin() + cursor.position.x, Glyph( c, palette.Default ) );

		cursor.position.x++;

		AdjustCursors( file, i, -1, 0 );
	}

	ScrollToCursor( file );
}

void TextEdit::MoveUp( File& file, bool shift )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.position.y == 0 ) continue;

		if( c.selectionOrigin == Coordinate( -1, -1 ) && shift )
		{
			c.selectionEnd = c.selectionOrigin = c.position;
		}

		c.position.y--;

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x > ( int )line.size() ) c.position.x = ( int )line.size();

		if( shift )
		{
			if( c.position < c.selectionOrigin )
			{
				c.selectionStart = c.position;
				c.selectionEnd   = c.selectionOrigin;
			}
			else
			{
				c.selectionEnd   = c.position;
				c.selectionStart = c.selectionOrigin;
			}

			Cursor* champ  = &c;
			int     offset = 0;
			while( Cursor* other = IsCoordinateInSelection( file, champ->position, offset ) )
			{
				if( other == champ )
				{
					offset++;
					continue;
				}
				other->selectionEnd = other->selectionOrigin = champ->selectionEnd;

				champ->disabled = true;
				champ           = other;
			}

			continue;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
		c.selectionOrigin                 = { -1, -1 };
	}

	DeleteDisabledCursor( file );
	YeetDuplicateCursors( file );
	ScrollToCursor( file );
}

void TextEdit::MoveDown( File& file, bool shift )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.position.y == ( int )file.Lines.size() - 1 ) continue;

		if( c.selectionOrigin == Coordinate( -1, -1 ) && shift )
		{
			c.selectionStart = c.selectionOrigin = c.position;
		}

		c.position.y++;

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x > ( int )line.size() ) c.position.x = ( int )line.size();

		if( shift )
		{
			if( c.position < c.selectionOrigin )
			{
				c.selectionStart = c.position;
				c.selectionEnd   = c.selectionOrigin;
			}
			else
			{
				c.selectionEnd   = c.position;
				c.selectionStart = c.selectionOrigin;
			}

			Cursor* champ  = &c;
			int     offset = 0;
			while( Cursor* other = IsCoordinateInSelection( file, champ->position, offset ) )
			{
				if( other == champ )
				{
					offset++;
					continue;
				}
				other->selectionStart = other->selectionOrigin = champ->selectionStart;

				champ->disabled = true;
				champ           = other;
			}

			continue;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
		c.selectionOrigin                 = { -1, -1 };
	}

	DeleteDisabledCursor( file );
	YeetDuplicateCursors( file );
	ScrollToCursor( file );
}

void TextEdit::MoveRight( File& file, bool ctrl, bool shift )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.selectionOrigin == Coordinate( -1, -1 ) && shift )
		{
			c.selectionStart = c.selectionOrigin = c.position;
		}

		Line& line = file.Lines [ c.position.y ];

		if( c.position.x == ( int )line.size() && c.position.y != ( int )file.Lines.size() )
		{
			c.position.x = 0;
			c.position.y++;
			ctrl = false;
		}
		else if( !ctrl )
		{
			c.position.x++;
		}

		if( ctrl )
		{
			Coordinate start;
			Coordinate end;
			GetWordAt( file, c.position, &start, &end );

			if( c.position.x == end.x ) c.position.x++;
			else
				c.position.x = end.x;
		}

		if( shift )
		{
			if( c.position > c.selectionOrigin )
			{
				c.selectionStart = c.selectionOrigin;
				c.selectionEnd   = c.position;
			}
			else
			{
				c.selectionEnd   = c.selectionOrigin;
				c.selectionStart = c.position;
			}

			Cursor* champ  = &c;
			int     offset = 0;
			while( Cursor* other = IsCoordinateInSelection( file, champ->position, offset ) )
			{
				if( other == champ )
				{
					offset++;
					continue;
				}
				other->selectionStart = other->selectionOrigin = champ->selectionStart;

				champ->disabled = true;
				champ           = other;
			}

			continue;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
		c.selectionOrigin                 = { -1, -1 };
	}

	DeleteDisabledCursor( file );
	YeetDuplicateCursors( file );
	ScrollToCursor( file );
}

void TextEdit::MoveLeft( File& file, bool ctrl, bool shift )
{
	for( int i = 0; i < file.cursors.size(); i++ )
	{
		Cursor& c = file.cursors [ i ];

		if( c.selectionOrigin == Coordinate( -1, -1 ) && shift )
		{
			c.selectionEnd = c.selectionOrigin = c.position;
		}

		int lineSize = -1;

		if( c.position.x == 0 && c.position.y != 0 )
		{
			Line& line   = file.Lines [ --c.position.y ];
			c.position.x = lineSize = ( int )line.size();
		}
		else if( !ctrl )
		{
			c.position.x--;
		}

		if( ctrl && lineSize == -1 )
		{
			Coordinate start;
			Coordinate end;
			GetWordAt( file, c.position, &start, &end );

			if( c.position.x == start.x )
			{
				if( c.position.x != 0 )
				{
					c.position.x--;
					GetWordAt( file, c.position, &start, &end );

					c.position.x = start.x;
				}
			}
			else
			{
				c.position.x = start.x;
			}
		}

		if( shift )
		{
			if( c.position > c.selectionOrigin )
			{
				c.selectionStart = c.selectionOrigin;
				c.selectionEnd   = c.position;
			}
			else
			{
				c.selectionEnd   = c.selectionOrigin;
				c.selectionStart = c.position;
			}

			Cursor* champ  = &c;
			int     offset = 0;
			while( Cursor* other = IsCoordinateInSelection( file, champ->position, offset ) )
			{
				if( other == champ )
				{
					offset++;
					continue;
				}
				other->selectionEnd = other->selectionOrigin = champ->selectionEnd;

				champ->disabled = true;
				champ           = other;
			}

			continue;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
		c.selectionOrigin                 = { -1, -1 };
	}

	DeleteDisabledCursor( file );
	YeetDuplicateCursors( file );
	ScrollToCursor( file );
}
