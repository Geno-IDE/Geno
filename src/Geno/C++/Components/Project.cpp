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

#include "Auxiliary/jsonSerializer.h"
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

FileFilter::FileFilter( std::string Name )
	: INode( {}, std::move( Name ), NodeKind::FileFilter )
{
} // FileFilter

//////////////////////////////////////////////////////////////////////////

void FileFilter::Rename( std::string Name )
{
	m_Name = std::move( Name );
	m_pParent->SortChildren();

} // Rename

//////////////////////////////////////////////////////////////////////////

void FileFilter::NewFile( std::filesystem::path Location, std::string Name )
{
	if( !ChildByName( Name ) )
	{
		std::ofstream OutputFile( Location / Name, std::ios::binary | std::ios::trunc );
		if( OutputFile.is_open() )
		{
			AddChild( new File( std::move( Location ), std::move( Name ) ) );
			OutputFile.close();
		}
	}

} // NewFile

//////////////////////////////////////////////////////////////////////////

Project::Project( std::filesystem::path Location, std::string Name )
	: INode( std::move( Location ), std::move( Name ), NodeKind::Project )
{
	AddChild( new FileFilter( "" ) ); // Create Default Empty FileFilter

} // Project

//////////////////////////////////////////////////////////////////////////

bool Project::Serialize( void )
{
	UTF8Converter UTF8;

	if( m_Location.empty() )
	{
		std::cerr << "Failed to serialize ";

		if( m_Name.empty() ) std::cerr << "unnamed project.";
		else
			std::cerr << "project '" << m_Name << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	jsonSerializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );

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

	// FileFilters
	{
		Serializer.Object( "FileFilters", [ & ]( void )
			{
				std::vector< std::string > Temp;
				for( const INode* rNode : m_pChildren )
					SerializeChildren( rNode, Serializer, Temp );
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

static void DeserializeChildren( const rapidjson::Value::ConstMemberIterator& rIt, INode* pNode )
{
	for( auto It = rIt->value.MemberBegin(); It < rIt->value.MemberEnd(); ++It )
	{
		const std::string MemberName = It->name.GetString();

		if( MemberName == "Files" )
		{
			for( auto ArrayIt = It->value.GetArray().Begin(); ArrayIt < It->value.GetArray().End(); ++ArrayIt )
			{
				const std::filesystem::path FilePath = std::filesystem::path( ArrayIt->GetString() );
				pNode->AddChild( new File( FilePath.parent_path(), FilePath.filename().string() ) );
			}
		}
		else
		{
			pNode->AddChild( new FileFilter( MemberName ) );
			DeserializeChildren( It, pNode->m_pChildren[ pNode->m_pChildren.size() - 1 ] );
		}
	}

} // DeserializeChildren

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

			if( ProjectKind == "Application" )        { m_ProjectKind = Kind::Application;    }
			else if( ProjectKind == "StaticLibrary" ) { m_ProjectKind = Kind::StaticLibrary;  }
			else if( ProjectKind == "DynamicLibrary" ){ m_ProjectKind = Kind::DynamicLibrary; }
			else                                      { m_ProjectKind = Kind::Unspecified;    }
		}
		else if( MemberName == "FileFilters" )
		{
			for( auto FiltersIt = It->value.MemberBegin(); FiltersIt < It->value.MemberEnd(); ++FiltersIt )
			{
				const std::string FilterName = FiltersIt->name.GetString();

				if( FilterName != "Default" )
					AddChild( new FileFilter( std::move( FilterName ) ) );

				DeserializeChildren( FiltersIt, m_pChildren[ m_pChildren.size() - 1 ] );
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
		if (rChildNode->m_Kind == NodeKind::FileFilter)
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
