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
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////

enum class NodeKind
{
	Workspace,
	Project,
	FileFilter,
	File,
	None
};

//////////////////////////////////////////////////////////////////////////

class INode;
class ICompiler;

//////////////////////////////////////////////////////////////////////////

// Pair.first = Index of Removed Node in m_pChildren
// Pair.second = Removed Node to be added back at the index in m_pChildren
inline std::vector< std::pair< uint32_t, INode* > > RemovedNodes;

void DeleteRemovedNodes( void );
void AddRemovedNode( INode*& pParentNode, const std::string& rName );

//////////////////////////////////////////////////////////////////////////

class INode
{
public:

	INode( std::filesystem::path Location, std::string Name, NodeKind Kind );
	virtual ~INode();

//////////////////////////////////////////////////////////////////////////

	virtual void Rename( std::string Name ) = 0;

//////////////////////////////////////////////////////////////////////////

	INode* ChildByName( const std::string& rName );
	void   SortChildren( void );
	void   AddChild( INode* pChild );
	void   RemoveChild( const std::string& rName );

	bool operator==( INode*& rNode );

//////////////////////////////////////////////////////////////////////////

	NodeKind m_Kind = NodeKind::None;

	std::string           m_Name;
	std::filesystem::path m_Location;

	INode*                m_pParent   = nullptr;
	std::vector< INode* > m_pChildren = {};

	bool m_ExpandNode = false; // Only To Be Used In WorkspaceOutliner.cpp

}; // INode
