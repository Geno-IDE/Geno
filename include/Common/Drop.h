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
#include <filesystem>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class Drop
{
public:

    struct Bitmap
    {
        std::unique_ptr< uint8_t[] > data;
        uint32_t                     width;
        uint32_t                     height;

    }; // Bitmap

    enum class TypeIndex
    {
        Text,
        Bitmap,
//		Audio,
        Paths,

    }; // TypeIndex

//////////////////////////////////////////////////////////////////////////

    using Text    = std::wstring;
//	using Audio   = std::nullptr_t;
    using Paths   = std::vector< std::filesystem::path >;
    using Variant = std::variant< Text, Bitmap, /*Audio,*/ Paths >;

//////////////////////////////////////////////////////////////////////////

    TypeIndex     GetType  ( void ) const    {
        return static_cast< TypeIndex >( m_Variant.index() );
    }
    const Text&   GetText  ( void ) const    {
        return std::get< Text >( m_Variant );
    }
    void          SetText  ( Text Text )     {
        m_Variant = std::move( Text );
    }
    const Bitmap& GetBitmap( void ) const    {
        return std::get< Bitmap >( m_Variant );
    }
    void          SetBitmap( Bitmap Bitmap ) {
        m_Variant = std::move( Bitmap );
    }
//	const Audio&  GetAudio ( void ) const    { return std::get< Audio >( m_Variant ); }
//	void          SetAudio ( Audio Audio )   { m_Variant = std::move( Audio ); }
    const Paths&  GetPaths ( void ) const    {
        return std::get< Paths >( m_Variant );
    }
    void          SetPaths ( Paths Paths )   {
        m_Variant = std::move( Paths );
    }

//////////////////////////////////////////////////////////////////////////

private:

    Variant m_Variant;

}; // Drop
