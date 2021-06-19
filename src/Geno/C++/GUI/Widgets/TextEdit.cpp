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

#define _CRT_SECURE_NO_WARNINGS
#include "TextEdit.h"

#include "Common/Drop.h"
#include "Common/LocalAppData.h"
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <fstream>
#include <iostream>

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

const char* WINDOW_NAME = "Text Edit";

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

	palette.Default				= 0xFFf4f4f4;
	palette.Keyword				= 0xFF0000F0;
	palette.Number				= 0xFF303030;
	palette.String				= 0xFF9E5817;
	palette.Comment				= 0xFF0f5904;
	palette.LineNumber			= 0xFFF0F0F0;
	palette.Selection			= 0x80a06020;
	palette.CurrentLine			= 0x40000000;
	palette.CurrentLineInactive = 0x40808080;
	palette.CurrentLineEdge		= 0x40a0a0a0;

	state.cursors.resize(1);
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
						else                                 ImageSize = ImVec2( ImageMaxSize * ( rBitmap.width / static_cast< float >( rBitmap.height ) ), ImageMaxSize );

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Image( m_DraggedBitmapTexture.GetID(), ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Text:
					{
						const Drop::Text& rText = pDrop->GetText();

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();
						ImGui::Text( "%ws", rText.c_str() );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Paths:
					{
						const Drop::Paths& rPaths = pDrop->GetPaths();

						ImGui::SetNextWindowPos( ImVec2( DragX, DragY ) );
						ImGui::BeginTooltip();

						for( const std::filesystem::path& rPath : rPaths )
							ImGui::BulletText( "%ws", rPath.c_str() );

						ImGui::EndTooltip();

					} break;
				}
			}
		}
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, BackgroundColor );
	ImGui::SetNextWindowSize( ImVec2( 350, 196 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( WINDOW_NAME, pOpen, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar) )
	{
		const int           TabBarFlags  = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_IsFocused;
		const ImGuiContext* pContext     = ImGui::GetCurrentContext();
		const ImGuiWindow*  pWindow      = ImGui::GetCurrentWindow();
		const ImRect        TabBarBounds = ImRect( pWindow->DC.CursorPos.x, pWindow->DC.CursorPos.y, pWindow->WorkRect.Max.x, pWindow->DC.CursorPos.y + pContext->FontSize + pContext->Style.FramePadding.y * 2 );

		if( !m_Files.empty() && ImGui::BeginTabBarEx( m_pTabBar, TabBarBounds, TabBarFlags, nullptr ) )
		{
			for( File& rFile : m_Files )
			{
				const std::string FileString = rFile.Path.filename().string();

				if( ImGui::BeginTabItem( FileString.c_str(), &rFile.Open ) )
				{
					const int InputTextFlags = ImGuiInputTextFlags_AllowTabInput;

					ImGui::PushFont( MainWindow::Instance().GetFontMono() );

					/*if( ImGui::InputTextMultiline( "##TextEditor", &rFile.Text, ImVec2( -0.01f, -0.01f ), InputTextFlags ) )
					{
						std::ofstream ofs( rFile.Path, std::ios::binary | std::ios::trunc );
						ofs << rFile.Text;
					}*/

					RenderEditor(rFile);

					ImGui::PopFont();
					ImGui::EndTabItem();
				}
			}

			// Clear closed files from list
			for( auto It = m_Files.begin(); It != m_Files.end(); )
			{
				if( It->Open ) It++;
				else           It = m_Files.erase( It );
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

	for( size_t i = 0; i < m_Files.size(); ++i )
	{
		File& rFile = m_Files[ i ];

		// Do not need to add file to vector if it already exists
		if( rFile.Path == rPath )
		{
			// Select the tab that corresponds to the open file
			m_pTabBar->NextSelectedTabId = m_pTabBar->Tabs[ static_cast< int >( i ) ].ID;

			// Update text in case file changed externally
			rFile.Text = Text;

			SplitLines(rFile);

			return;
		}
	}

	File File;
	File.Path = rPath;
	File.Text = Text;

	SplitLines(File);

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

				for( const std::filesystem::path& rPath : rPaths )
					AddFile( rPath );

			} break;
		}
	}

} // OnDragDrop

void TextEdit::SplitLines(File& file) {


	file.Lines.clear();

	Line lineBuffer;

	for (unsigned int i = 0; i < file.Text.length(); i++) {
		const char c = file.Text[i];

		if (c == '\n') {
			file.Lines.push_back(lineBuffer);
			lineBuffer.clear();
		} else {
			lineBuffer.push_back(Glyph(c, palette.Default));
		}
	}

	file.Lines.push_back(lineBuffer);
}

bool TextEdit::RenderEditor(File& file) {
	state.currentFile = &file;
	props.changes = false;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(0xFF101010));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	const float fontSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "#").x;
	props.charAdvanceY = ImGui::GetTextLineHeightWithSpacing();

	ImGui::PushAllowKeyboardFocus(true);

	HandleKeyboardInputs();

	unsigned int totalLines = (unsigned int)file.Lines.size();

	char buf[16];
	sprintf(buf, " %u | ", totalLines);

	const float lineNumMaxWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;

	memset(buf, 0, sizeof(buf));

	ImVec2 size = ImGui::GetContentRegionMax();
	ImVec2 cursor = ImGui::GetCursorScreenPos();


	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::SetCursorScreenPos(ImVec2(cursor.x + lineNumMaxWidth, cursor.y));
	ImGui::BeginChild("##TextEditor", ImVec2(size.x - lineNumMaxWidth, 0), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar );

	HandleMouseInputs();

	props.scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

	unsigned int firstLine = (unsigned int)(props.scroll.y / props.charAdvanceY);
	unsigned int lastLine = std::min(firstLine + (unsigned int)(size.y / props.charAdvanceY + 2), totalLines - 1);

	float longest = 0.0f;

	for (unsigned int i = firstLine; i <= lastLine; i++) {
		ImVec2 pos(cursor.x + lineNumMaxWidth - props.scroll.x, cursor.y + (i - firstLine) * props.charAdvanceY);
		Line& line = file.Lines[i];

		Coordinate selectedStart;
		Coordinate selectedEnd;

		if (IsLineSelected(i, &selectedStart, &selectedEnd)) {
			ImVec2 start(cursor.x + lineNumMaxWidth + GetDistance(selectedStart), pos.y);
			ImVec2 end(cursor.x + lineNumMaxWidth + GetDistance(selectedEnd), pos.y + props.charAdvanceY);

			drawList->AddRectFilled(start, end, palette.Selection);
		}

		for (unsigned int j = 0; j < state.cursors.size(); j++) {
			Cursor c = state.cursors[j];

			if (c.disabled) continue;

			if (c.position.y == i) {

				bool focus = ImGui::IsWindowFocused();

				if (!HasSelection(j))  {
					ImVec2 start(cursor.x + lineNumMaxWidth, pos.y);
					ImVec2 end(cursor.x + size.x, pos.y + props.charAdvanceY);

					drawList->AddRectFilled(start, end, focus ? palette.CurrentLine : palette.CurrentLineInactive);
					drawList->AddRect(start, end, palette.CurrentLineEdge);
				}



				if (focus) {
					static auto start = std::chrono::system_clock::now();
					auto now = std::chrono::system_clock::now();
					long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

					if (elapsed >= cursorBlink) {
						elapsed -= cursorBlink;

						float cursorPos = GetCursorDistance(j);
						ImVec2 cStart(pos.x + cursorPos, pos.y);
						ImVec2 cEnd(cStart.x + 1.0f, cStart.y + props.charAdvanceY - 1);

						drawList->AddRectFilled(cStart, cEnd, palette.Cursor);

						if (elapsed >= cursorBlink)
							start = now;
					}
				}
			}
		}

		std::string stringBuffer;

		float xOffset = 0.0f;
		unsigned int prevColor = line.size() ? line[0].color : palette.Default;

		for (Glyph& glyph : line) {

			if (glyph.color != prevColor) {
				drawList->AddText(ImVec2(pos.x + xOffset, pos.y), prevColor, stringBuffer.c_str());
				float textWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, stringBuffer.c_str()).x;
				xOffset += textWidth;
				stringBuffer.clear();

				prevColor = glyph.color;
			} else {
				stringBuffer.push_back(glyph.c);
			}
		}

		if (!stringBuffer.empty()) {
			drawList->AddText(ImVec2(pos.x + xOffset, pos.y), prevColor, stringBuffer.c_str());
			float textWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, stringBuffer.c_str()).x;
			xOffset += textWidth;
			stringBuffer.clear();
		}

		if (xOffset > longest) longest = xOffset;


	}



	ImGui::Dummy(ImVec2(longest + 10, (totalLines + 10) * props.charAdvanceY));

	ImGui::PopAllowKeyboardFocus();
	ImGui::EndChild();


	//Render line numbers
	ImGui::SetCursorScreenPos(ImVec2(cursor.x-2, cursor.y));
	ImGui::BeginChild("##LineNumbers", ImVec2(lineNumMaxWidth, size.y + 2), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	for (unsigned int i = firstLine; i <= lastLine; i++) {
		sprintf(buf, "%u | ", i + 1);

		const float currentLineNumWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;
		ImVec2 pos(cursor.x + lineNumMaxWidth - currentLineNumWidth, cursor.y + (i - firstLine) * props.charAdvanceY);

		drawList->AddText(pos, palette.LineNumber, buf);
	}

	ImGui::EndChild();


	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	state.currentFile = nullptr;

	return props.changes;
}

void TextEdit::HandleKeyboardInputs() {
	ImGuiIO& io = ImGui::GetIO();

	bool shift = io.KeyShift;
	bool ctrl = io.KeyCtrl;
	bool alt = io.KeyAlt;

	// Keyboard Inputs
	io.WantCaptureKeyboard = true;
	io.WantTextInput = true;

	if (!shift && !ctrl & !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
		EnterTextStuff(ImGuiKey_Enter);
	else if (!shift && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
		EnterTextStuff(ImGuiKey_Backspace);
	else if (!shift && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
		MoveUp();
	else if (!shift && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
		MoveDown();
	else if (!shift && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
		MoveRight();
	else if (!shift && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
		MoveLeft();

	for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
		char c = (char)io.InputQueueCharacters[i];

		EnterTextStuff(c);
	}
}

void TextEdit::HandleMouseInputs() {
	ImGuiIO& io = ImGui::GetIO();

	bool shift = io.KeyShift;
	bool ctrl = io.KeyCtrl;
	bool alt = io.KeyAlt;

	if (ImGui::IsWindowHovered()) {
			// Mouse Inputs
		ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

		bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool doubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		bool dragged = ImGui::IsMouseDragging(ImGuiMouseButton_Left);

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || props.changes) {
			state.cursors[state.cursors.size() - 1].selectionOrigin = props.changes ? GetMouseCoordinate() : Coordinate(~0, ~0);


			for (unsigned int i = 0; i < state.cursors.size(); i++) {
				if (state.cursors[i].disabled) state.cursors.erase(state.cursors.begin() + i--);
			}
		}

		if (doubleClicked) {
			Cursor& lastCursor = state.cursors[state.cursors.size() - 1];

			std::string word = GetWordAt(lastCursor);

			if (word.empty()) return;

			lastCursor.position = lastCursor.selectionEnd;

		} else if (clicked) {
			if (ctrl && !(alt || shift)) {

			} else {
				Cursor c;
				c.position = c.selectionOrigin = GetMouseCoordinate();

				if (!(ctrl && alt)) state.cursors.clear();

				if (!IsCoordinateInSelection(c.position, true))
					state.cursors.push_back(c);
			}
		} else if (dragged) {
			Coordinate pos = GetMouseCoordinate();
			Cursor& cursor = state.cursors[state.cursors.size() - 1];

			if (cursor.selectionOrigin != Coordinate(~0, ~0)) {
				if (pos > cursor.selectionOrigin) {
					cursor.selectionEnd = pos;
					cursor.selectionStart = cursor.selectionOrigin;
				} else {
					cursor.selectionStart = pos;
					cursor.selectionEnd = cursor.selectionOrigin;
				}

				cursor.position = pos;

				DisableIntersectingSelections(state.cursors.size() - 1);
			}
		}
	}
}

bool TextEdit::HasSelection(unsigned int cursor) const {
	GENO_ASSERT(cursor < state.cursors.size());

	const Cursor& c = state.cursors[cursor];

	if (c.disabled) return false;

	if (c.selectionStart.y != c.selectionEnd.y)
		return c.selectionEnd.y > c.selectionStart.y;

	return c.selectionEnd.x > c.selectionStart.x;
}

bool TextEdit::IsCoordinateInSelection(Coordinate coordinate, bool includePosition) {

	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];

		if ((coordinate > c.selectionStart && coordinate < c.selectionEnd) || (coordinate == c.position))
			return true;
	}

	return false;
}

bool TextEdit::IsLineSelected(unsigned int line, Coordinate* start, Coordinate* end) const {
	GENO_ASSERT(state.currentFile != nullptr);

	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		const Cursor& c = state.cursors[i];

		if (c.selectionStart == c.selectionEnd || c.disabled) continue;

		if (line == c.selectionStart.y) {
			*start = c.selectionStart;

			if (line == c.selectionEnd.y) {
				*end = c.selectionEnd;
			} else {
				end->y = line;
				end->x = state.currentFile->Lines[line].size();
			}

			return true;
		} else if (line >= c.selectionStart.y && line <= c.selectionEnd.y) {
			start->x = 0;
			start->y = line;

			if (line == c.selectionEnd.y) {
				*end = c.selectionEnd;
			} else {
				end->y = line;
				end->x = state.currentFile->Lines[line].size();
			}

			return true;
		}
	}


	return false;
}

float TextEdit::GetCursorDistance(unsigned int cursor) const {
	GENO_ASSERT(cursor < state.cursors.size());
	return GetDistance(state.cursors[cursor].position);
}

float TextEdit::GetDistance(Coordinate position) const {
	GENO_ASSERT(state.currentFile != nullptr);

	const Line& line = state.currentFile->Lines[position.y];

	char* string = new char[position.x + 1];

	string[position.x] = 0;

	for (unsigned int i = 0; i < position.x; i++) {
		string[i] = line[i].c;
	}

	float res = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, string).x;

	delete[] string;

	return res;
}

std::string TextEdit::GetWordAt(Cursor& cursor) const {
	return GetWordAt(cursor.position, &cursor.selectionStart, &cursor.selectionEnd);
}

std::string TextEdit::GetWordAt(Coordinate position, Coordinate* start, Coordinate* end) const {
	GENO_ASSERT(state.currentFile != nullptr);

	const Line& l = state.currentFile->Lines[position.y];

	if (position.x >= l.size()) return std::string();

	char c = l[position.x].c;

	auto cmp = [](char c) -> bool { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); };

	if (cmp(c)) {
		std::string buffer;

		unsigned int len = l.size();
		unsigned int x0 = 0, x1 = len;

		for (unsigned int i = position.x+1; i < len; i++) {
			char chr = l[i].c;
			if (!cmp(chr)) {
				x1 = i;
				break;
			}

			buffer.push_back(chr);
		}

		for (unsigned int i = position.x; i > 0; i--) {
			char chr = l[i].c;
			if (!cmp(chr)) {
				x0 = i+1;
				break;
			}

			buffer.insert(buffer.begin(), chr);
		}

		if (start) *start = { x0, position.y };
		if (end) *end = { x1, position.y };

		return std::move(buffer);
	}

	return std::string();
}

void TextEdit::SetSelectionLine(unsigned int line) {
	GENO_ASSERT(state.currentFile != nullptr);
	if (line >= state.currentFile->Lines.size()) return;

	Line& l = state.currentFile->Lines[line];

	Coordinate start(0, line);
	Coordinate end(l.size(), line);

	SetSelection(start, end, 0);
}

void TextEdit::SetSelection(Coordinate start, Coordinate end, unsigned int cursor) {
	GENO_ASSERT(state.currentFile != nullptr);
	GENO_ASSERT(start.x < end.x && start.y < end.y);
	GENO_ASSERT(cursor < state.cursors.size());

	auto& lines = state.currentFile->Lines;

	if (start.y >= lines.size()) return;

	Line& firstLine = lines[start.y];

	if (end.y >= lines.size()) {
		end.y = lines.size() - 1;

		end.x = lines[end.y].size();
	}

	Cursor& c = state.cursors[cursor];

	c.selectionStart = start;
	c.selectionEnd = end;

}

TextEdit::Coordinate TextEdit::GetMouseCoordinate(float* distance) {
	GENO_ASSERT(state.currentFile != nullptr);
	ImVec2 origin = ImGui::GetCursorScreenPos();
	ImVec2 mousePos = ImGui::GetMousePos();

	mousePos.x -= origin.x;
	mousePos.y -= origin.y;

	unsigned int line = (unsigned int)((mousePos.y + props.scroll.y) / props.charAdvanceY) - (props.scroll.y / props.charAdvanceY);

	unsigned int numLines = state.currentFile->Lines.size();

	if (line > numLines - 1) {
		line = numLines - 1;
	}

	const Line& l = state.currentFile->Lines[line];

	unsigned int lineSize = (unsigned int)l.size();

	char* string = new char[lineSize + 1];
	memset(string, 0, lineSize + 1);

	float length = 0.0f;

	for (unsigned int i = 0; i < l.size(); i++) {
		string[i] = l[i].c;

		float currLength = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, string).x;
		float diff = currLength - length;

		if (currLength - (diff / 2.0f) > mousePos.x) {
			delete[] string;
			if (distance) *distance = length;
			return Coordinate(i, line);
		}

		length = currLength;
	}

	delete[] string;

	if (distance) *distance = length;

	return Coordinate(l.size(), line);
}

void TextEdit::AdjustCursors(unsigned int cursor, unsigned int xOffset, unsigned int yOffset) {
	Cursor& c = state.cursors[cursor];
	for (unsigned int j = 0; j < state.cursors.size(); j++) {
		if (j == cursor) continue;

		Cursor& other = state.cursors[j];

		if (other.selectionStart > c.selectionStart) {
			if (other.selectionStart.y == c.selectionStart.y) {
				other.selectionStart.x -= c.selectionStart.x;

				if (other.selectionEnd.y == c.selectionStart.y) {
					other.selectionStart.x -= c.selectionStart.x;
				}
			} else {
				other.selectionStart.y -= yOffset;
				other.selectionEnd.y -= yOffset;
			}
		}

		if (other.position > c.position) {
			if (other.position.y == c.position.y) {
				other.position.x -= xOffset;
			}

			other.position.y -= yOffset;
		}
	}
}

void TextEdit::YeetDuplicateCursors() {
	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];
		for (unsigned int j = i+1; j < state.cursors.size(); j++) {
			Cursor& tmpC = state.cursors[j];

			if (tmpC.position == c.position) {
				state.cursors.erase(state.cursors.begin() + j);
			}
		}
	}
}

void TextEdit::DisableIntersectingSelections(unsigned int cursor) {
	Cursor& c = state.cursors[cursor];

	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		if (i == cursor) continue;

		Cursor& c2 = state.cursors[i];

		if ((c2.selectionStart > c.selectionStart && c2.selectionStart < c.selectionEnd) || (c2.selectionEnd > c.selectionStart && c2.selectionEnd < c.selectionEnd)) {
			c2.disabled = true;
		} else {
			c2.disabled = false;
		}
	}
}

void TextEdit::Enter(unsigned int cursor) {
	Cursor& c = state.cursors[cursor];

	auto& lines = state.currentFile->Lines;

	Line newLine;
	Line& line = lines[c.position.y];

	bool ass = c.position.x != line.size();

	if (ass) {
		auto start = line.begin() + c.position.x;
		newLine.insert(newLine.begin(), start, line.end());
		line.erase(start, line.end());
	}

	AdjustCursors(cursor, c.position.x, ass ? -1 : 0);

	c.position.y++;
	c.position.x = 0;
	lines.insert(lines.begin() + c.position.y, newLine);
}

void TextEdit::Backspace(unsigned int cursor) {
	GENO_ASSERT(state.currentFile != nullptr);

	Cursor& c = state.cursors[cursor];

	auto& lines = state.currentFile->Lines;

	if (HasSelection(cursor)) {
		Line& l = lines[c.selectionStart.y];

		unsigned int yOffset = 0;
		unsigned int xOffset = 0;

		if (c.selectionEnd.y == c.selectionStart.y) {
			l.erase(l.begin() + c.selectionStart.x, l.begin() + c.selectionEnd.x);

			xOffset = c.selectionEnd.x - c.selectionStart.x;
		} else {
			l.erase(l.begin() + c.selectionStart.x, l.end());

			int numLines = (int)c.selectionEnd.y - c.selectionStart.y - 1;

			if (numLines > 0) {
				lines.erase(lines.begin() + c.selectionStart.y + 1, lines.begin() + c.selectionStart.y + numLines + 1);
			}

			Line& l2 = lines[c.selectionStart.y + 1];

			yOffset = numLines + 1;
			xOffset = c.selectionEnd.x - 1;

			if (xOffset == ~0) xOffset++;

			l2.erase(l2.begin(), l2.begin() + xOffset);

			if (!l2.empty()) {
				l.insert(l.begin(), l2.begin(), l2.end());
			}

			lines.erase(lines.begin() + c.selectionStart.y + 1);
		}


		AdjustCursors(cursor, xOffset, yOffset);

		c.position = c.selectionStart;

		c.selectionStart = { 0, 0 };
		c.selectionEnd = { 0, 0 };
	} else {
		Line& line = lines[c.position.y];

		if (c.position.x == 0 && c.position.y != 0) {
			Line& lineAbove = lines[c.position.y - 1];

			unsigned int x = lineAbove.size();

			if (!line.empty()) {
				lineAbove.insert(lineAbove.end(), line.begin(), line.end());
			}

			lines.erase(lines.begin() + c.position.y);

			AdjustCursors(cursor, -x, 1);

			c.position.x = x;
			c.position.y--;
		} else if (!(c.position.y == 0 && c.position.x == 0)) {
			c.position.x--;
			line.erase(line.begin() + c.position.x);

			AdjustCursors(cursor, 1, 0);
		}

		YeetDuplicateCursors();

	}

}

void TextEdit::EnterTextStuff(char c) {
	GENO_ASSERT(state.currentFile != nullptr);

	props.changes = true;

	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& cursor = state.cursors[i];

		if (HasSelection(i))
			Backspace(i);

		if (c == ImGuiKey_Enter) {
			Enter(i);
			continue;
		} else if (c == ImGuiKey_Backspace) {
			Backspace(i);
			continue;
		}

		Line& l = state.currentFile->Lines[cursor.position.y];

		l.insert(l.begin() + cursor.position.x, Glyph(c, palette.Default));

		cursor.position.x++;

		AdjustCursors(i, -1, 0);
	}

}

void TextEdit::MoveUp() {
	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];

		if (c.position.y == 0) continue;

		c.position.y--;

		Line& line = state.currentFile->Lines[c.position.y];

		if (c.position.x > line.size()) c.position.x = line.size();

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors();
}

void TextEdit::MoveDown() {
	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];

		if (c.position.y == state.currentFile->Lines.size()-1) continue;

		c.position.y++;

		Line& line = state.currentFile->Lines[c.position.y];

		if (c.position.x > line.size()) c.position.x = line.size();

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors();
}

void TextEdit::MoveRight() {
	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];

		Line& line = state.currentFile->Lines[c.position.y];

		if (c.position.x == line.size() && c.position.y != state.currentFile->Lines.size()) {
			c.position.x = 0;
			c.position.y++;
		} else {
			c.position.x++;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors();
}

void TextEdit::MoveLeft() {
	for (unsigned int i = 0; i < state.cursors.size(); i++) {
		Cursor& c = state.cursors[i];

		if (c.position.x == 0 && c.position.y != 0) {
			Line& line = state.currentFile->Lines[--c.position.y];
			c.position.x = line.size();
		} else {
			c.position.x--;
		}

		c.selectionStart = c.selectionEnd = { 0, 0 };
	}

	YeetDuplicateCursors();
}
