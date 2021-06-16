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

class  Drop;
struct ImGuiTabBar;

class TextEdit
{
public:
	struct Palette {
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

	struct Glyph {
		char			c;
		unsigned int	color;

		Glyph(char c, unsigned int color) : c(c), color(color) {}
	};

	typedef std::vector<Glyph> Line;

	struct File
	{
		std::filesystem::path Path;
		std::string           Text;

		std::vector<Line>	  Lines;

		bool                  Open = true;

	}; // File

	struct Coordinate {
		unsigned int x;
		unsigned int y;

		Coordinate() : x(0), y(0) {}
		Coordinate(unsigned int x, unsigned int y) : x(x) , y(y) {}
	};

	struct Cursor {
		Coordinate position;

		float distance;
	};

	struct State {
		Coordinate selectionStart;
		Coordinate selectionEnd;
		std::vector<Cursor> cursors;

		File* currentFile;
	} state;

//////////////////////////////////////////////////////////////////////////

	TextEdit( void );

//////////////////////////////////////////////////////////////////////////

	void Show      ( bool* pOpen );
	void AddFile   ( const std::filesystem::path& rPath );
	void OnDragDrop( const Drop& rDrop, int X, int Y );

//////////////////////////////////////////////////////////////////////////

private:

	struct Properties {
		float charAdvanceY;
		ImVec2 scroll;
	} props;

	void SplitLines(File& file);
	bool RenderEditor(File& file);
	void HandleInputs();
	bool HasSelection() const;
	float GetCursorDistance(unsigned int cursor) const;
	void SetSelectionLine(unsigned int line);
	void SetSelection(Coordinate start, Coordinate end);
	Coordinate GetMouseCoordinate(float* distance = nullptr);

	Palette palette;

	unsigned int cursorBlink = 500;


//////////////////////////////////////////////////////////////////////////

	ImGuiTabBar*        m_pTabBar              = nullptr;

	Texture2D           m_DraggedBitmapTexture = { };

	std::vector< File > m_Files                = { };

}; // TextEdit
