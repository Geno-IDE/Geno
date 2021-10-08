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



#include "SDFFont.h"

#include "Common/Macros.h"
#include "Common/Profiling.h"

#include <msdf-atlas-gen/ImmediateAtlasGenerator.h>
#include <msdf-atlas-gen/TightAtlasPacker.h>
#include <msdf-atlas-gen/Charset.h>
#include <msdf-atlas-gen/FontGeometry.h>
#include <msdf-atlas-gen/glyph-generators.h>
#include <msdf-atlas-gen/BitmapAtlasStorage.h>
#include <msdf-atlas-gen/GlyphGeometry.h>
#include <msdfgen/ext/import-font.h>

#include <GL/glew.h>

#include <iostream>
#include <thread>

using namespace msdf_atlas;
using namespace msdfgen;

SDFFont::SDFFont(const std::string& rFilename) : m_FontFilename(rFilename), m_AtlasWidth(-1), m_AtlasHeight(-1) {
	Timer timer("FONT");
	FreetypeHandle* pFreetypeHandle = initializeFreetype();
	FontHandle* pFontHandle = loadFont(pFreetypeHandle, rFilename.c_str());

	if (pFontHandle == nullptr) {
		GENO_ASSERT(false);
	}

	Charset CS = Charset::ASCII;
	std::vector<GlyphGeometry> Glyphs;

	FontGeometry Geometry(&Glyphs);

	int GlyphsLoaded = Geometry.loadCharset(pFontHandle, 1, CS, false, false);

	if (GlyphsLoaded <= 0) {
		GENO_ASSERT(false);
	}

	TightAtlasPacker Packer;

	Packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE);
	Packer.setPadding(0);
	Packer.setScale(32);
	Packer.setPixelRange(2);
	Packer.setUnitRange(0);
	Packer.setMiterLimit(1);

	int Remaining = Packer.pack(Glyphs.data(), (int)Glyphs.size());

	if (Remaining != 0) {
		GENO_ASSERT(false);
	}

	Packer.getDimensions(m_AtlasWidth, m_AtlasHeight);

	for (GlyphGeometry& rGlyph : Glyphs) {
		rGlyph.edgeColoring(msdfgen::edgeColoringInkTrap, 3, 0);
	}

	GeneratorAttributes Attribs;

	Attribs.scanlinePass = true;
	Attribs.config.errorCorrection.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;

	ImmediateAtlasGenerator<float, 3, msdfGenerator, BitmapAtlasStorage<byte, 3>> Generator(m_AtlasWidth, m_AtlasHeight);

	Generator.setAttributes(Attribs);
	Generator.setThreadCount(std::thread::hardware_concurrency());
	Generator.generate(Glyphs.data(), (int)Glyphs.size());

	BitmapConstRef<byte, 3> Bitmap = Generator.atlasStorage();

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_AtlasWidth, m_AtlasHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, Bitmap.pixels);

	destroyFont(pFontHandle);
	deinitializeFreetype(pFreetypeHandle);

	for (GlyphGeometry& rGlyph : Glyphs) {
		int x, y, w, h;
		double l, r, t, b;

		rGlyph.getBoxRect(x, y, w, h);
		rGlyph.getQuadPlaneBounds(l, b, r, t);

		float Left = (float)x / (float)m_AtlasWidth;
		float Right = (float)(x + w)/ (float)m_AtlasWidth;
		float Top = (float)y / (float)m_AtlasHeight;
		float Bottom = (float)(y + h) / (float)m_AtlasHeight;

		Glyph g;

		g.C = (char)rGlyph.getCodepoint();
		g.TexTopLeft = ImVec2(Left, Bottom);
		g.TexTopRight = ImVec2(Right, Bottom);
		g.TexBottomLeft = ImVec2(Left, Top);
		g.TexBottomRight = ImVec2(Right, Top);

		m_Glyphs.push_back(g);
	}
}

SDFFont::~SDFFont() {
	glDeleteTextures(1, &m_TextureID);
}

const SDFFont::Glyph* SDFFont::GetGlyph(char C) {
	for (Glyph& rGlyph : m_Glyphs) {
		if (rGlyph.C == C) return &rGlyph;
	}

	return nullptr;
}

unsigned int SDFFont::GetAtlasID() {
	return m_TextureID;
}
