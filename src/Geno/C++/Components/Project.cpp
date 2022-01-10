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

#include "Project.h"

#include "Compilers/ICompiler.h"
#include "GUI/Widgets/StatusBar.h"

#include <fstream>
#include <iostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////

File::File( std::filesystem::path Location, std::string Name )
	: INode( std::move( Location ), std::move( Name ), NodeKind::File )
{
} // File

//////////////////////////////////////////////////////////////////////////

void File::Rename( std::string Name )
{
	const std::filesystem::path OldPath = m_Location / m_Name;

	if( std::filesystem::exists( OldPath ) )
	{
		const std::filesystem::path NewPath = m_Location / Name;
		std::filesystem::rename( OldPath, NewPath );
	}

	m_Name = std::move( Name );
	m_pParent->SortChildren();

} // Rename

//////////////////////////////////////////////////////////////////////////

Group::Group( std::filesystem::path Location, std::string Name, bool WorkspaceGroup )
	: INode( std::move( Location ), std::move( Name ), NodeKind::Group )
	, m_WorkspaceGroup( std::move( WorkspaceGroup ) )
{
} // Group

//////////////////////////////////////////////////////////////////////////

void Group::Rename( std::string Name )
{
	m_Name = std::move( Name );
	m_pParent->SortChildren();

} // Rename

//////////////////////////////////////////////////////////////////////////

void Group::Serialize( JSONSerializer& rSerializer )
{
	rSerializer.Object( m_Name == "" ? "DefaultGroup" : m_Name, [ & ]()
		{
			std::vector< std::string > Array;
			std::string                ArrayName;

			for( INode*& rNode : m_pChildren )
			{
				if( rNode->m_Kind == NodeKind::Group )
				{
					Group* pGroup = ( Group* )( rNode );
					pGroup->Serialize( rSerializer );
				}
				else if( rNode->m_Kind == NodeKind::Project )
				{
					if( ArrayName.empty() ) { ArrayName = "Projects"; }

					Project* pProject = ( Project* )( rNode );

					if( pProject->Serialize() )
						Array.push_back( ( rNode->m_Location.lexically_relative( m_Location ) / rNode->m_Name ).string() );
				}
				else if( rNode->m_Kind == NodeKind::File )
				{
					if( ArrayName.empty() ) { ArrayName = "Files"; }
					Array.push_back( ( rNode->m_Location.lexically_relative( m_Location ) / rNode->m_Name ).string() );
				}
			}

			if( !Array.empty() )
				rSerializer.Add( std::move( ArrayName ), std::move( Array ) );
		} );

} // Serialize

//////////////////////////////////////////////////////////////////////////

void Group::Deserialize( const rapidjson::Value::ConstMemberIterator& rIt )
{
	for( auto GroupIt = rIt->value.MemberBegin(); GroupIt < rIt->value.MemberEnd(); ++GroupIt )
	{
		const std::string MemberName = GroupIt->name.GetString();

		if( MemberName == "Files" || MemberName == "Projects" )
		{
			for( auto ArrayIt = GroupIt->value.GetArray().Begin(); ArrayIt < GroupIt->value.GetArray().End(); ++ArrayIt )
			{
				std::filesystem::path Path = ArrayIt->GetString();

				if( !Path.is_absolute() )
					Path = m_Location / Path;
				Path = Path.lexically_normal();

				if( MemberName == "Files" )
				{
					if( std::filesystem::exists( Path ) )
						AddChild( new File( Path.parent_path(), Path.filename().string() ) );
				}
				else if( MemberName == "Projects" )
				{
					Project* pProject = new Project( Path.parent_path(), Path.stem().string() );
					if( pProject->Deserialize() )
						AddChild( std::move( pProject ) );
					else
						delete pProject;
				}
			}
		}
		else
		{
			Group* pGroup = new Group( m_Location, GroupIt->name.GetString(), m_WorkspaceGroup );
			pGroup->Deserialize( GroupIt );
			AddChild( std::move( pGroup ) );
		}
	}

} // Deserialize

//////////////////////////////////////////////////////////////////////////

Project::Project( std::filesystem::path Location, std::string Name )
	: INode( std::move( Location ), std::move( Name ), NodeKind::Project )
{
	AddChild( new Group( m_Location, "" ) ); // Create Default Empty Group

} // Project

//////////////////////////////////////////////////////////////////////////

bool Project::Serialize( void )
{
	UTF8Converter UTF8;

	if( m_Location.empty() )
	{
		std::cerr << "Failed to serialize ";

		if( m_Name.empty() ) std::cerr << "unnamed project.";
		else                 std::cerr << "project '" << m_Name << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	JSONSerializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );

	// Kind
	{
		std::string KindString = "";

		switch( m_ProjectKind )
		{
			case Kind::Application:    KindString = "Application";    break;
			case Kind::StaticLibrary:  KindString = "StaticLibrary";  break;
			case Kind::DynamicLibrary: KindString = "DynamicLibrary"; break;
			default:                   KindString = "Unspecified";    break;
		}

		Serializer.Add( "Kind", std::move( KindString ) );
	}

	// Groups
	{
		Serializer.Object( "Groups", [ & ]( void )
			{
				for( INode*& rNode : m_pChildren )
				{
					Group* pGroup = ( Group* )rNode;
					pGroup->Serialize( Serializer );
				}
			} );
	}

	// Include Directories
	if( !m_LocalConfiguration.m_IncludeDirs.empty() )
	{
		std::vector< std::string > Dirs;
		for( const std::filesystem::path& rIncludeDir : m_LocalConfiguration.m_IncludeDirs )
		{
			const std::filesystem::path RelativePath = rIncludeDir.lexically_relative( m_Location );
			Dirs.push_back( RelativePath.string() );
		}

		Serializer.Add( "IncludeDirs", std::move( Dirs ) );
	}

	// Library Directories
	if( !m_LocalConfiguration.m_LibraryDirs.empty() )
	{
		std::vector< std::string > Dirs;
		for( const std::filesystem::path& rLibraryDir : m_LocalConfiguration.m_LibraryDirs )
		{
			const std::filesystem::path RelativePath = rLibraryDir.lexically_relative( m_Location );
			Dirs.push_back( RelativePath.string() );
		}

		Serializer.Add( "LibraryDirs", std::move( Dirs ) );
	}

	// Preprocessor defines
	if( !m_LocalConfiguration.m_Defines.empty() )
	{
		Serializer.Add( "Defines", m_LocalConfiguration.m_Defines );
	}

	// Libraries
	if( !m_LocalConfiguration.m_Libraries.empty() )
	{
		Serializer.Add( "Libraries", m_LocalConfiguration.m_Libraries );
	}

	return true;

} // Serialize

//////////////////////////////////////////////////////////////////////////

bool Project::Deserialize( void )
{
	if( m_Location.empty() )
	{
		std::cerr << "Failed to deserialize ";

		if( m_Name.empty() ) std::cerr << "unnamed project.";
		else
			std::cerr << "project '" << m_Name << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	if( !std::filesystem::exists( ( m_Location / m_Name ).replace_extension( EXTENSION ) ) )
		return false;

	rapidjson::Document Doc;

	std::ifstream     gprj( ( m_Location / m_Name ).replace_extension( EXTENSION ), std::ios::in );
	std::stringstream Content;
	Content << gprj.rdbuf();
	gprj.close();
	Doc.Parse( Content.str().c_str() );

	for( auto It = Doc.MemberBegin(); It < Doc.MemberEnd(); ++It )
	{
		const std::string MemberName = It->name.GetString();

		if( MemberName == "Kind" )
		{
			const std::string ProjectKind = It->value.GetString();

			if     ( ProjectKind == "Application" )   { m_ProjectKind = Kind::Application;    }
			else if( ProjectKind == "StaticLibrary" ) { m_ProjectKind = Kind::StaticLibrary;  }
			else if( ProjectKind == "DynamicLibrary" ){ m_ProjectKind = Kind::DynamicLibrary; }
			else                                      { m_ProjectKind = Kind::Unspecified;    }
		}
		else if( MemberName == "Groups" )
		{
			for( auto GroupsIt = It->value.MemberBegin(); GroupsIt < It->value.MemberEnd(); ++GroupsIt )
			{
				const std::string GroupName = GroupsIt->name.GetString();

				if( GroupName == "DefaultGroup" )
				{
					Group* pGroup = ( Group* )m_pChildren[ 0 ];
					pGroup->Deserialize( GroupsIt );
				}
				else
				{
					Group* pGroup = new Group( m_Location, GroupName );
					pGroup->Deserialize( GroupsIt );
					AddChild( std::move( pGroup ) );
				}
			}
		}
		else if( MemberName == "IncludeDirs" )
		{
			const auto Array = It->value.GetArray();
			for( auto i = Array.Begin(); i < Array.End(); ++i )
			{
				std::filesystem::path Path = i->GetString();

				if( !Path.is_absolute() )
					Path = m_Location / Path;

				Path = Path.lexically_normal();
				m_IncludeDirectories.push_back( Path );
			}
		}
		else if( MemberName == "LibraryDirs" )
		{
			const auto Array = It->value.GetArray();
			for( auto i = Array.Begin(); i < Array.End(); ++i )
			{
				std::filesystem::path Path = i->GetString();

				if( !Path.is_absolute() )
					Path = m_Location / Path;

				Path = Path.lexically_normal();
				m_LibraryDirectories.push_back( Path );
			}
		}
		else if( MemberName == "Defines" )
		{
			const auto Array = It->value.GetArray();
			for( auto i = Array.Begin(); i < Array.End(); ++i )
			{
				m_Defines.push_back( i->GetString() );
			}
		}
		else if( MemberName == "Libraries" )
		{
			const auto Array = It->value.GetArray();
			for( auto i = Array.Begin(); i < Array.End(); ++i )
			{
				m_Libraries.push_back( i->GetString() );
			}
		}
	}

	return true;

} // Deserialize

//////////////////////////////////////////////////////////////////////////

void Project::Rename( std::string Name )
{
	const std::filesystem::path OldPath = ( m_Location / m_Name ).replace_extension( Project::EXTENSION );

	if( std::filesystem::exists( OldPath ) )
	{
		const std::filesystem::path NewPath = ( m_Location / Name ).replace_extension( Project::EXTENSION );
		std::filesystem::rename( OldPath, NewPath );
	}

	m_Name = std::move( Name );
	m_pParent->SortChildren();
	Serialize();

} // Rename

//////////////////////////////////////////////////////////////////////////

static void FindSourceFoldersInChildren( INode*& rNode, std::vector< std::filesystem::path >& rSourcePaths )
{
	if( !rNode ) { return; }

	for (INode*& rChildNode : rNode->m_pChildren)
	{
		if (rChildNode->m_Kind == NodeKind::Group)
		{
			FindSourceFoldersInChildren( rChildNode, rSourcePaths );
		}
		else if(rChildNode->m_Kind == NodeKind::File)
		{
			// Path already found.
			if( std::find( rSourcePaths.begin(), rSourcePaths.end(), rChildNode->m_Location ) != rSourcePaths.end() )
				break;

			auto Extension = ( rChildNode->m_Location / rChildNode->m_Name ).extension();

			// #TODO: We really need a function that does this.
			if(
				Extension == ".cc"
				|| Extension == ".cpp"
				|| Extension == ".cxx"
				|| Extension == ".c++"
				|| Extension == ".h"
				|| Extension == ".hh"
				|| Extension == ".hpp"
				|| Extension == ".hxx"
				|| Extension == ".h++" )
			{
				rSourcePaths.push_back( rChildNode->m_Location );
			}
		}
	}
} // FindSourceFoldersInChildren

//////////////////////////////////////////////////////////////////////////

std::vector< std::filesystem::path > Project::FindSourceFolders( void )
{
	// Walk through all files and get the parent path. And check if that path does not exist already.
	std::vector<std::filesystem::path> SourcePaths;

	for(INode*& rNode : m_pChildren)
	{
		FindSourceFoldersInChildren( rNode, SourcePaths );
	}

	return SourcePaths;

} // FindSourceFolders
