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

#include "IWidget.h"

#include <imgui.h>

IWidget::IWidget( const std::filesystem::path& rWidgetFile )
	: m_WidgetFile( rWidgetFile )
{
} // IWidget

//////////////////////////////////////////////////////////////////////////

void IWidget::Observe()
{
	for( auto&& [ rCombination, rActionName ] : m_KeyBindings )
	{
		bool CallAction = false;
		for( int i = 0; i < ( int )( rCombination.size() - 1 ); i++ )
		{
			CallAction = ImGui::IsKeyDown( rCombination[ i ] );
		}

		if( !CallAction && rCombination.size() != 1 )
			continue;

		CallAction = ImGui::IsKeyPressed( rCombination[ rCombination.size() - 1 ] );

		if( CallAction )
			m_Actions[ rActionName ]();
	}

} // Observe

//////////////////////////////////////////////////////////////////////////

void IWidget::WriteKeyBindings( GCL::Serializer& rSerializer )
{
	rSerializer.StartObject( "KeyBindings" );

	for( auto&& [ rCombination, rActionName ] : m_KeyBindings )
	{
		rSerializer.Write( rActionName, rCombination );
	}

	rSerializer.EndObject();

} // WriteKeyBindings

//////////////////////////////////////////////////////////////////////////

void IWidget::ReadKeyBindings( GCL::Member& rMember )
{
	auto Members = rMember.GetValue< std::vector< GCL::Member > >();
	for( GCL::Member& rObjectMember : Members )
	{
		m_KeyBindings.insert( { rObjectMember.GetValue< std::vector< int > >(), rObjectMember.Key } );
	}

} // ReadKeyBindings
