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

#include "Framebuffer.h"
#include "GUI/SDFFont.h"
#include "GUI/Widgets/TextEdit.h"

class SDFRenderer {
public:
	SDFRenderer();
	~SDFRenderer();

	void Resize(int Width, int Height);

	void StartFrame(int LineNumMaxWidth, float CharSize);
	void SubmitLine(int Line, const TextEdit::Line& rGlyphs);
	void SubmitCharacter(int X, int Y, char C, int Color);
	void EndFrame();
	void Render();

	void SetFont(SDFFont* pFont);

	unsigned int GetTextureID();

private:
	bool CompileShaders(const char* Vert, const char* Frag);

private:


	struct CharData {
		struct {
			ImVec2 TopLeft;
			ImVec2 TopRight;
			ImVec2 BottomLeft;
			ImVec2 BottomRight;
		} TexCoords;

		ImVec2 Position;
		unsigned int Color;
	};

	unsigned int m_ShaderID;
	unsigned int m_VboID;
	unsigned int m_VboCharID;
	unsigned int m_VaoID;

	unsigned int m_ScaleUniform;
	unsigned int m_ProjectionUniform;

	unsigned int m_SubmitCount;
	unsigned int m_CurrentLine;
	unsigned int m_LineNumMaxWidth;
	float m_CharSize;

	std::vector<CharData> m_CharData;

	float m_Projection[16];

	Framebuffer* m_pFramebuffer;
	SDFFont* m_pFont;
};
