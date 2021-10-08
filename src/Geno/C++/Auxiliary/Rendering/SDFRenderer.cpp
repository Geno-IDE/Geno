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

#include "SDFRenderer.h"
#include "Common/Macros.h"

#include <GL/glew.h>
#include <string>
#include <iostream>

const char* Vertex =
#include "SDFVert.glsl"
;

const char* Fragment =
#include "SDFFrag.glsl"
;

SDFRenderer::SDFRenderer() : m_ShaderID(0), m_SubmitCount(1) {
	if (!CompileShaders(Vertex, Fragment)) {
		return;
	}

	float Vertices[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};

	glGenBuffers(1, &m_VboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &m_VboCharID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboCharID);
	glBufferData(GL_ARRAY_BUFFER, 4096, 0, GL_DYNAMIC_DRAW); // Pre allocate 1024

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_VaoID);
	glBindVertexArray(m_VaoID);
	// Main buffer with initial vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, m_VboID);

	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 8, 0);

	// Char data per instance
	glBindBuffer(GL_ARRAY_BUFFER, m_VboCharID);

	// TexCoord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(CharData), (const void*)offsetof(CharData, TexCoords.TopLeft));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(CharData), (const void*)offsetof(CharData, TexCoords.TopRight));
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(CharData), (const void*)offsetof(CharData, TexCoords.BottomLeft));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, false, sizeof(CharData), (const void*)offsetof(CharData, TexCoords.BottomRight));
	glVertexAttribDivisor(4, 1);

	// POsition
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, false, sizeof(CharData), (const void*)offsetof(CharData, Position));
	glVertexAttribDivisor(5, 1);

	// Color
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, true, sizeof(CharData), (const void*)offsetof(CharData, Color));
	glVertexAttribDivisor(6, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	m_pFramebuffer = new Framebuffer(100, 100);

	memset(m_Projection, 0, sizeof(m_Projection));
}

SDFRenderer::~SDFRenderer() {
	glDeleteProgram(m_ShaderID);
	glDeleteBuffers(1, &m_VboID);
	glDeleteBuffers(1, &m_VboCharID);
	glDeleteVertexArrays(1, &m_VaoID);

	delete m_pFramebuffer;
}

void SDFRenderer::Resize(int Width, int Height) {
	m_pFramebuffer->Resize(Width, Height);

	float L = 0;
	float R = (float)Width;
	float T = 0;
	float B = (float)Height;
	float F = 1.0f;
	float N = -0.01f;

	m_Projection[0] = 2.0f / (R - L);
	m_Projection[1 * 4 + 1] = 2.0f / (T - B);
	m_Projection[2 * 4 + 2] = -2.0f / (F - N);
	m_Projection[3 * 4 + 0] = -(R + L) / (R - L);
	m_Projection[3 * 4 + 1] = -(T + B) / (T - B);
	m_Projection[3 * 4 + 2] = -(F + N) / (F - N);
	m_Projection[3 * 4 + 3] = 1.0f;

	glProgramUniformMatrix4fv(m_ShaderID, m_ProjectionUniform, 1, false, m_Projection);
}

void SDFRenderer::StartFrame( int LineNumMaxWidth, float CharSize ) {
	m_LineNumMaxWidth = LineNumMaxWidth;
	m_SubmitCount = 0;
	m_CurrentLine = 0;
	m_CharSize = CharSize;

	m_CharData.clear();
}

void SDFRenderer::SubmitLine(int Line, const TextEdit::Line& rGlyphs) {
	int xOffset = 0;
	(void)Line;
	for (const TextEdit::Glyph& rGlyph : rGlyphs) {
		SubmitCharacter(xOffset++, m_CurrentLine, rGlyph.C, rGlyph.Color);
	}

}

void SDFRenderer::SubmitCharacter(int X, int Y, char C, int Color) {
	const SDFFont::Glyph* pGlyph = m_pFont->GetGlyph(C);

	if (pGlyph == nullptr) {
		pGlyph = m_pFont->GetGlyph('?');

		GENO_ASSERT(pGlyph != nullptr);
	}

	CharData Data;

	Data.Position = ImVec2((float)X, (float)Y);
	Data.Color = Color;
	Data.TexCoords.TopLeft = pGlyph->TexTopLeft;
	Data.TexCoords.TopRight = pGlyph->TexTopRight;
	Data.TexCoords.BottomLeft = pGlyph->TexBottomLeft;
	Data.TexCoords.BottomRight = pGlyph->TexBottomRight;

	m_CharData.push_back(Data);

	m_SubmitCount++;
}

void SDFRenderer::EndFrame() {
	glNamedBufferSubData(m_VboCharID, 0, m_CharData.size() * sizeof(CharData), m_CharData.data());
}

void SDFRenderer::Render() {
	EndFrame();

	m_pFramebuffer->BindRenderTarget();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_ShaderID);
	glBindVertexArray(m_VaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pFont->GetAtlasID());

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_SubmitCount);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	m_pFramebuffer->UnBindRenderTarget();
}

void SDFRenderer::SetFont(SDFFont* pFont) {
	m_pFont = pFont;

}

unsigned int SDFRenderer::GetTextureID() {
	return m_pFramebuffer->GetTextureID();
}

bool SDFRenderer::CompileShaders(const char* VertSrc, const char* FragSrc) {
	unsigned int Vert = glCreateShader(GL_VERTEX_SHADER);
	unsigned int Frag = glCreateShader(GL_FRAGMENT_SHADER);

	int VLen = (int)strlen(VertSrc);
	int FLen = (int)strlen(FragSrc);

	glShaderSource(Vert, 1, &VertSrc, &VLen);
	glShaderSource(Frag, 1, &FragSrc, &FLen);

	glCompileShader(Vert);
	glCompileShader(Frag);

	char Log[1024];

	int Status = 0;

	glGetShaderiv(Vert, GL_COMPILE_STATUS, &Status);

	if (Status == GL_FALSE) {
		glGetShaderInfoLog(Vert, 1024, 0, Log);
		std::cout << "Failed to compile vertex shader: " << Log << "\n";
		glDeleteShader(Vert);
		glDeleteShader(Frag);
		return false;
	}

	glGetShaderiv(Frag, GL_COMPILE_STATUS, &Status);

	if (Status == GL_FALSE) {
		glGetShaderInfoLog(Frag, 1024, 0, Log);
		std::cout << "Failed to compile fragment shader: " << Log << "\n";
		glDeleteShader(Vert);
		glDeleteShader(Frag);
		return false;
	}

	m_ShaderID = glCreateProgram();

	glAttachShader(m_ShaderID, Vert);
	glAttachShader(m_ShaderID, Frag);

	glLinkProgram(m_ShaderID);

	glDeleteShader(Vert);
	glDeleteShader(Frag);

	glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &Status);

	if (Status == GL_FALSE) {
		glGetProgramInfoLog(m_ShaderID, 1024, 0, Log);
		std::cout << "Failed to link shader: " << Log << "\n";
		glDeleteProgram(m_ShaderID);
		m_ShaderID = 0;
		return false;
	}

	glValidateProgram(m_ShaderID);

	glGetProgramiv(m_ShaderID, GL_VALIDATE_STATUS, &Status);

	if (Status == GL_FALSE) {
		glGetProgramInfoLog(m_ShaderID, 1024, 0, Log);
		std::cout << "Failed to validate shader: " << Log << "\n";
		glDeleteProgram(m_ShaderID);
		m_ShaderID = 0;
		return false;
	}

	m_ScaleUniform = glGetUniformLocation(m_ShaderID, "Scale");
	m_ProjectionUniform = glGetUniformLocation(m_ShaderID, "Projection");

	return true;

}
