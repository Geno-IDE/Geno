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

class Framebuffer {
public:
	Framebuffer(int Width, int Height);
	~Framebuffer();

	void Resize(int Width, int Height);

	void BindRenderTarget();
	void UnBindRenderTarget();

	int GetFramebufferID();
	int GetTextureID();
	int GetWidth();
	int GetHeight();

private:
	unsigned int m_FramebufferID;
	unsigned int m_TextureID;
	int m_Width;
	int m_Height;
};
