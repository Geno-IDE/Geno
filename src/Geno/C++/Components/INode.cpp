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

#include "INode.h"

inline unsigned int NodeIdCounter = 0;

INode::INode( std::filesystem::path Location, std::string Name, NodeKind Kind )
	: m_Kind( std::move( Kind ) )
	, m_Name( std::move( Name ) )
	, m_Location( std::move( Location ) )
{
	m_pChildren.resize( 0 );
	m_Id = ++NodeIdCounter;

} // INode

//////////////////////////////////////////////////////////////////////////

INode::~INode()
{
	for( INode*& rNode : m_pChildren )
	{
		if( rNode )
		{
			delete rNode;
			rNode = nullptr;
		}
	}

} // ~INode

//////////////////////////////////////////////////////////////////////////

static bool AlphabeticCompare( std::string_view a, std::string_view b )
{
	if( a.empty() )
	{
		return false;
	}

	size_t len = std::min( a.size(), b.size() );
	for( size_t i = 0; i < len; i++ )
	{
		char CharA = a[ i ];
		char CharB = b[ i ];
		if( std::isalpha( CharA ) && std::isalpha( CharB ) )
		{
			char LowerCharA = std::tolower( CharA, std::locale() );
			char LowerCharB = std::tolower( CharB, std::locale() );
			if( LowerCharA == LowerCharB )
			{
				if( CharA > CharB )
				{
					return true;
				}
				else if( CharA < CharB )
				{
					return false;
				}
			}
			else if( LowerCharA < LowerCharB )
			{
				return true;
			}
			else if( LowerCharA > LowerCharB )
			{
				return false;
			}
		}
		else if( CharA < CharB )
		{
			return true;
		}
		else if( CharA > CharB )
		{
			return false;
		}
	}

	return a.size() < b.size();

} // AlphabeticCompare

//////////////////////////////////////////////////////////////////////////

void INode::SortChildren( void )
{
	std::sort( m_pChildren.begin(), m_pChildren.end(), []( INode* a, INode* b )
		{ return AlphabeticCompare( a->m_Name, b->m_Name ); } );

} // SortChildren

//////////////////////////////////////////////////////////////////////////

INode* INode::ChildByName( const std::string& rName )
{
	for( INode*& rChild : m_pChildren )
	{
		if( rChild->m_Name == rName )
			return rChild;
	}

	return nullptr;

} // ChildByName

//////////////////////////////////////////////////////////////////////////

void INode::AddChild( INode* pChild )
{
	pChild->m_pParent = this;
	m_pChildren.push_back( std::move( pChild ) );
	SortChildren();

} // AddChild

//////////////////////////////////////////////////////////////////////////

void INode::RemoveChild( const std::string& rName )
{
	for( uint32_t i = 0; i < ( uint32_t )m_pChildren.size(); ++i )
	{
		if( m_pChildren[ i ] && m_pChildren[ i ]->m_Name == rName )
		{
			RemovedNodes.push_back( { i, m_pChildren[ i ] } );
			m_pChildren[ i ] = nullptr;
		}
	}

} // RemoveChild

//////////////////////////////////////////////////////////////////////////

bool INode::operator==( INode*& rNode )
{
	return m_Id == rNode->m_Id;

} // operator==

//////////////////////////////////////////////////////////////////////////

void DeleteRemovedNodes( void )
{
	for( auto& rRemovedNode : RemovedNodes )
	{
		if( rRemovedNode.second )
			delete rRemovedNode.second;
	}

} // DeleteRemovedNodes

//////////////////////////////////////////////////////////////////////////

void AddRemovedNode( INode*& pParentNode, const std::string& rName )
{
	for( auto& rRemovedNode : RemovedNodes )
	{
		INode* pNode = rRemovedNode.second;

		if( pNode )
		{
			if( pNode->m_Name == rName && pNode->m_pParent == pParentNode )
			{
				pParentNode->m_pChildren[ rRemovedNode.first ] = pNode;
				break;
			}
		}
	}

} // AddRemovedNode
