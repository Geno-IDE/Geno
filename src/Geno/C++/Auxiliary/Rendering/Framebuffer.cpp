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

#include "Framebuffer.h"
#include "Common/Macros.h"

#include <GL/glew.h>

Framebuffer::Framebuffer(int Width, int Height) : m_Width(Width), m_Height(Height) {
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &m_FramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		GENO_ASSERT(false);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
	glDeleteFramebuffers(1, &m_FramebufferID);
	glDeleteTextures(1, &m_TextureID);
}

void Framebuffer::Resize(int Width, int Height) {
	if (m_Width == Width && m_Height == Height) return;

	m_Width = Width;
	m_Height = Height;

	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::BindRenderTarget() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
	glViewport(0, 0, m_Width, m_Height);
}

void Framebuffer::UnBindRenderTarget() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int Framebuffer::GetFramebufferID() {
	return m_FramebufferID;
}

int Framebuffer::GetTextureID() {
	return m_TextureID;
}

int Framebuffer::GetWidth() {
	return m_Width;
}

int Framebuffer::GetHeight() {
	return m_Height;
}

