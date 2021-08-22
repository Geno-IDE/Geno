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

#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>
#include "GUI/Widgets/StatusBar.h"

#include <fstream>
#include <iostream>

//////////////////////////////////////////////////////////////////////////

Project::Project( std::filesystem::path Location )
	: m_Location( std::move( Location ) )
	, m_Name    ( "MyProject" )
{
	NewFileFilter( "" ); // Create Default Empty FileFilter

} // Project

//////////////////////////////////////////////////////////////////////////

Project::Project( Project&& rrOther )
{
	*this = std::move( rrOther );

} // Project

//////////////////////////////////////////////////////////////////////////

Project& Project::operator=( Project&& rrOther )
{
	m_Kind               = rrOther.m_Kind;
	m_Location           = std::move( rrOther.m_Location );
	m_Name               = std::move( rrOther.m_Name );
	m_FileFilters        = std::move( rrOther.m_FileFilters );
	m_IncludeDirectories = std::move( rrOther.m_IncludeDirectories );
	m_LibraryDirectories = std::move( rrOther.m_LibraryDirectories );
	m_Defines            = std::move( rrOther.m_Defines );
	m_Libraries          = std::move( rrOther.m_Libraries );
	m_FilesLeftToBuild   = std::move( rrOther.m_FilesLeftToBuild );
	m_FilesToLink        = std::move( rrOther.m_FilesToLink );

	rrOther.m_Kind       = Kind::Unspecified;

	return *this;

} // operator=

//////////////////////////////////////////////////////////////////////////

void Project::Build( ICompiler& rCompiler )
{
	m_FilesLeftToBuild.clear();
	m_FilesToLink     .clear();

	if( m_FileFilters.empty() )
		return;

	StatusBar::Instance().SetText( "Build Started..." );

	for( const FileFilter& rFileFilter : m_FileFilters )
	{
		for( const std::filesystem::path& rFile : rFileFilter.Files )
		{
			std::filesystem::path Extension = rFile.extension();

			// #TODO: Compiler will be per-file so this check is only temporary
			if( Extension == ".cpp"
			 || Extension == ".cxx"
			 || Extension == ".cc"
			 || Extension == ".c" )
			{
				bool Found = false;
				for( const std::filesystem::path& rrFile : m_FilesLeftToBuild )
				{
					if( std::filesystem::equivalent( rFile, rrFile ) )
					{
						Found = true;
						break;
					}
				}

				if( !Found )
				{
					m_FilesLeftToBuild.push_back( rFile );
				}
			}
		}
	}

	BuildNextFile( rCompiler );

} // Build

//////////////////////////////////////////////////////////////////////////

bool Project::Serialize( void )
{
	if( m_Location.empty() )
	{
		std::cerr << "Failed to serialize ";

		if( m_Name.empty() ) std::cerr << "unnamed project.";
		else                 std::cerr << "project '" << m_Name << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	GCL::Serializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );
	if( !Serializer.IsOpen() )
		return false;

	// Name
	{
		GCL::Object Name( "Name" );
		Name.SetString( m_Name );
		Serializer.WriteObject( Name );
	}

	// Kind
	{
		GCL::Object Kind( "Kind" );

		switch( m_Kind )
		{
			case Kind::Application:    { Kind.SetString( "Application" );    } break;
			case Kind::StaticLibrary:  { Kind.SetString( "StaticLibrary" );  } break;
			case Kind::DynamicLibrary: { Kind.SetString( "DynamicLibrary" ); } break;
			default:                   { Kind.SetString( "Unspecified" );    } break;
		}

		Serializer.WriteObject( Kind );
	}

	// Filters
	if( !m_FileFilters.empty() )
	{
		GCL::Object Filters( "FileFilters", std::in_place_type< GCL::Object::TableType > );

		for( const FileFilter& rFileFilter : m_FileFilters )
		{
			if( !rFileFilter.Name.empty() )
			{
				GCL::Object FileFilter( rFileFilter.Name.string(), std::in_place_type< GCL::Object::TableType > );

				std::string FilterPathString = rFileFilter.Path.string();
				if( !FilterPathString.empty() )
				{
					GCL::Object FilterPath( "Path", std::in_place_type< GCL::Object::StringType > );
					FilterPath.SetString( FilterPathString );
					FileFilter.AddChild( std::move( FilterPath ) );
				}

				if( !rFileFilter.Files.empty() )
				{
					GCL::Object Files( "Files", std::in_place_type< GCL::Object::TableType > );
					for( const std::filesystem::path& rFile : rFileFilter.Files )
					{
						const std::filesystem::path RelativePath = rFile.lexically_relative( m_Location );

						Files.AddChild( GCL::Object( RelativePath.string() ) );
					}
					FileFilter.AddChild( std::move( Files ) );
				}

				Filters.AddChild( std::move( FileFilter ) );
			}
		}

		Serializer.WriteObject( Filters );
	}

	// Files
	if( FileFilter* pEmptyFileFilter = FileFilterByName( "" ) )
	{
		if( !pEmptyFileFilter->Files.empty() )
		{
			GCL::Object Files( "Files", std::in_place_type< GCL::Object::TableType > );

			for( const std::filesystem::path& rFile : pEmptyFileFilter->Files )
			{
				const std::filesystem::path RelativePath = rFile.lexically_relative( m_Location );

				Files.AddChild( GCL::Object( RelativePath.string() ) );
			}

			Serializer.WriteObject( Files );
		}
	}

	// Include directories
	if( !m_IncludeDirectories.empty() )
	{
		GCL::Object IncludeDirs( "IncludeDirs", std::in_place_type< GCL::Object::TableType > );

		for( const std::filesystem::path& rIncludeDir : m_IncludeDirectories )
		{
			const std::filesystem::path RelativePath = rIncludeDir.lexically_relative( m_Location );

			IncludeDirs.AddChild( GCL::Object( RelativePath.string() ) );
		}

		Serializer.WriteObject( IncludeDirs );
	}

	// Library directories
	if( !m_LibraryDirectories.empty() )
	{
		GCL::Object LibraryDirs( "LibraryDirs", std::in_place_type< GCL::Object::TableType > );

		for( const std::filesystem::path& rLibraryDir : m_LibraryDirectories )
		{
			const std::filesystem::path RelativePath = rLibraryDir.lexically_relative( m_Location );

			LibraryDirs.AddChild( GCL::Object( RelativePath.string() ) );
		}

		Serializer.WriteObject( LibraryDirs );
	}

	// Preprocessor defines
	if( !m_Defines.empty() )
	{
		GCL::Object Defines( "Defines", std::in_place_type< GCL::Object::TableType > );

		for( const std::string& rDefine : m_Defines )
		{
			Defines.AddChild( GCL::Object( rDefine ) );
		}

		Serializer.WriteObject( Defines );
	}

	// Libraries
	if( !m_Libraries.empty() )
	{
		GCL::Object Libraries( "Libraries", std::in_place_type< GCL::Object::TableType > );

		for( const std::string& rLibrary : m_Libraries )
		{
			Libraries.AddChild( GCL::Object( rLibrary ) );
		}

		Serializer.WriteObject( Libraries );
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
		else                 std::cerr << "project '" << m_Name << "'.";

		std::cerr << " Location not specified.\n";

		return false;
	}

	GCL::Deserializer Deserializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );
	if( !Deserializer.IsOpen() )
		return false;

	Deserializer.Objects( this, GCLObjectCallback );

	if( FileFilter* pEmptyFileFilter = FileFilterByName( "" ) )
	{
		auto it = pEmptyFileFilter->Files.begin();
		while( it != pEmptyFileFilter->Files.end() )
		{
			bool Found = false;
			for( FileFilter& rFileFilter : m_FileFilters )
			{
				if( rFileFilter.Name.empty() )
				{
					continue;
				}

				for( std::filesystem::path& rrFile : rFileFilter.Files )
				{
					if( std::filesystem::equivalent( *it, rrFile ) )
					{
						Found = true;
						break;
					}
				}

				if( Found )
				{
					break;
				}
			}

			if( Found )
			{
				it = pEmptyFileFilter->Files.erase( it );
			}
			else
			{
				it++;
			}
		}
	}

	FileFilter EmptyFileFilter = {};

	if( !EmptyFileFilter.Files.empty() )
	{
		m_FileFilters.emplace_back( std::move( EmptyFileFilter ) );
	}

	return true;

} // Deserialize

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
}

//////////////////////////////////////////////////////////////////////////

void Project::SortFileFilters( void )
{
	for( FileFilter& rFileFilter : m_FileFilters )
	{
		std::sort( rFileFilter.Files.begin(), rFileFilter.Files.end(), []( const std::filesystem::path& a, const std::filesystem::path& b )
			{
				return AlphabeticCompare( a.filename().string(), b.filename().string() );
			} );
	}

	std::sort( m_FileFilters.begin(), m_FileFilters.end(), []( const FileFilter& a, const FileFilter& b )
		{
			return AlphabeticCompare( a.Name.string(), b.Name.string() );
		} );

} // SortFileFilters

//////////////////////////////////////////////////////////////////////////

FileFilter* Project::NewFileFilter( const std::filesystem::path& Name )
{
	if( FileFilterByName( Name ) )
	{
		return nullptr;
	}

	FileFilter Filter;
	Filter.Name = Name;
	m_FileFilters.emplace_back( std::move( Filter ) );

	SortFileFilters();

	return FileFilterByName( Name );

} // NewFileFilter

//////////////////////////////////////////////////////////////////////////

void Project::RemoveFileFilter( const std::filesystem::path& Name )
{
	auto it = std::find_if( m_FileFilters.begin(), m_FileFilters.end(), [ & ]( const FileFilter& FileFilter ) -> bool
		{
			return FileFilter.Name == Name;
		} );

	if( it != m_FileFilters.end() )
	{
		m_FileFilters.erase( it );
	}
	SortFileFilters();

} // RemoveFileFilter

//////////////////////////////////////////////////////////////////////////

FileFilter* Project::FileFilterByName( const std::filesystem::path& Name )
{
	for( FileFilter& rFileFilter : m_FileFilters )
	{
		if( rFileFilter.Name == Name )
		{
			return &rFileFilter;
		}
	}

	return nullptr;

} // FileFilterFromName

//////////////////////////////////////////////////////////////////////////

std::filesystem::path Project::FileInFileFilter( const std::filesystem::path& rFile, const std::filesystem::path& rFileFilter )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		for( const std::filesystem::path& rrFile : pFileFilter->Files )
		{
			if( rFile == rrFile )
				return rrFile;
		}
	}
	return {};

} // FileInFileFilter

//////////////////////////////////////////////////////////////////////////

void Project::RenameFileFilter( const std::filesystem::path& rFileFilter, const std::string& rName )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		pFileFilter->Name = std::move( rName );
		SortFileFilters();
		Serialize();
	}

} // RenameFileFilter

//////////////////////////////////////////////////////////////////////////

bool Project::NewFile( const std::filesystem::path& rPath, const std::filesystem::path& rFileFilter )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		if( FileInFileFilter( rPath, rFileFilter ).empty() )
		{
			std::ofstream OutputFileStream( rPath, std::ios::binary | std::ios::trunc );

			if( OutputFileStream.is_open() )
			{
				pFileFilter->Files.emplace_back( rPath );
				SortFileFilters();
				Serialize();
				OutputFileStream.close();
				return true;
			}
		}
	}

	return false;

} // NewFile

//////////////////////////////////////////////////////////////////////////

bool Project::AddFile( const std::filesystem::path& rPath, const std::filesystem::path& rFileFilter )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		if( FileInFileFilter( rPath, rFileFilter ).empty() )
		{
			pFileFilter->Files.emplace_back( std::move( rPath ) );
			SortFileFilters();
			Serialize();
			return true;
		}
	}

	return false;

} // AddFile

//////////////////////////////////////////////////////////////////////////

void Project::RemoveFile( const std::filesystem::path& rFile, const std::filesystem::path& rFileFilter )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		for( auto It = pFileFilter->Files.begin(); It != pFileFilter->Files.end(); ++It )
		{
			if( *It == rFile )
			{
				pFileFilter->Files.erase( It );
				SortFileFilters();
				Serialize();
				break;
			}
		}
	}

} // RemoveFile

//////////////////////////////////////////////////////////////////////////

void Project::RenameFile( const std::filesystem::path& rFile, const std::filesystem::path& rFileFilter, const std::string& rName )
{
	if( FileFilter* pFileFilter = FileFilterByName( rFileFilter ) )
	{
		for( auto& rrFile : pFileFilter->Files )
		{
			if( rrFile == rFile )
			{
				const std::filesystem::path OldPath = rrFile;

				if( std::filesystem::exists( OldPath ) )
				{
					const std::filesystem::path NewPath = m_Location / pFileFilter->Path / rName;
					std::filesystem::rename( OldPath, NewPath );
				}

				rrFile = m_Location / pFileFilter->Path / rName;
				SortFileFilters();
				Serialize();
			}
		}
	}

} // RenameFile

//////////////////////////////////////////////////////////////////////////

void Project::GCLObjectCallback( GCL::Object Object, void* pUser )
{
	Project*         pSelf = static_cast< Project* >( pUser );
	std::string_view Name  = Object.Name();

	if( Name == "Name" )
	{
		pSelf->m_Name = Object.String();
	}
	else if( Name == "Kind" )
	{
		const std::string& rKindString = Object.String();

		if(      rKindString == "Application" )    { pSelf->m_Kind = Kind::Application; }
		else if( rKindString == "StaticLibrary" )  { pSelf->m_Kind = Kind::StaticLibrary; }
		else if( rKindString == "DynamicLibrary" ) { pSelf->m_Kind = Kind::DynamicLibrary; }
		else                                       { pSelf->m_Kind = Kind::Unspecified; }
	}
	else if( Name == "FileFilters" )
	{
		for( const GCL::Object& rFileFilterObj : Object.Table() )
		{
			FileFilter FileFilter;
			FileFilter.Name = rFileFilterObj.Name();

			for( const GCL::Object& rFileFilterObject : rFileFilterObj.Table() )
			{
				std::string_view FileFilterObjectName = rFileFilterObject.Name();

				if( FileFilterObjectName == "Path" )
				{
					FileFilter.Path = rFileFilterObject.String();
				}
				else if( FileFilterObjectName == "Files" )
				{
					for( const GCL::Object& rFilePathObj : rFileFilterObject.Table() )
					{
						std::filesystem::path FilePath = rFilePathObj.String();

						if( !FilePath.is_absolute() )
							FilePath = pSelf->m_Location / FilePath;

						FilePath = FilePath.lexically_normal();
						FileFilter.Files.emplace_back( std::move( FilePath ) );
					}
				}
			}

			pSelf->m_FileFilters.emplace_back( std::move( FileFilter ) );
		}
		pSelf->SortFileFilters();
	}
	else if( Name == "Files" )
	{
		for( const GCL::Object& rFilePathObj : Object.Table() )
		{
			std::filesystem::path FilePath = rFilePathObj.Name();

			if( !FilePath.is_absolute() )
				FilePath = pSelf->m_Location / FilePath;

			FilePath                = FilePath.lexically_normal();
			FileFilter* pFileFilter = pSelf->FileFilterByName( "" );
			if( !pFileFilter )
			{
				pFileFilter = pSelf->NewFileFilter( "" );
			}
			pFileFilter->Files.emplace_back( std::move( FilePath ) );
		}
	}
	else if( Name == "IncludeDirs" )
	{
		for( const GCL::Object& rFilePathObj : Object.Table() )
		{
			std::filesystem::path FilePath = rFilePathObj.Name();

			if( !FilePath.is_absolute() )
				FilePath = pSelf->m_Location / FilePath;

			FilePath = FilePath.lexically_normal();
			pSelf->m_IncludeDirectories.emplace_back( std::move( FilePath ) );
		}
	}
	else if( Name == "LibraryDirs" )
	{
		for( const GCL::Object& rFilePathObj : Object.Table() )
		{
			std::filesystem::path FilePath = rFilePathObj.Name();

			if( !FilePath.is_absolute() )
				FilePath = pSelf->m_Location / FilePath;

			FilePath = FilePath.lexically_normal();
			pSelf->m_LibraryDirectories.emplace_back( std::move( FilePath ) );
		}
	}
	else if( Name == "Defines" )
	{
		for( const GCL::Object& rDefineObj : Object.Table() )
		{
			std::string Define( rDefineObj.Name() );

			pSelf->m_Defines.emplace_back( std::move( Define ) );
		}
	}
	else if( Name == "Libraries" )
	{
		for( const GCL::Object& rLibraryObj : Object.Table() )
		{
			std::string Library( rLibraryObj.Name() );

			pSelf->m_Libraries.emplace_back( std::move( Library ) );
		}
	}

} // GCLObjectCallback

//////////////////////////////////////////////////////////////////////////

void Project::BuildNextFile( ICompiler& rCompiler )
{
	if( m_FilesLeftToBuild.empty() )
	{
		Link( rCompiler );

		return;
	}

//////////////////////////////////////////////////////////////////////////

	std::filesystem::path& File = m_FilesLeftToBuild.back();
	// Listen to every file compilation to check if we're done compiling
	rCompiler.Events.FinishedCompiling += [ this ]( ICompiler& rCompiler, CompileOptions Options, int /*ExitCode*/ )
	{
		if( auto NextFile = std::find( m_FilesLeftToBuild.begin(), m_FilesLeftToBuild.end(), Options.InputFile ); NextFile != m_FilesLeftToBuild.end() )
		{
			m_FilesToLink.push_back( Options.OutputFile );
			m_FilesLeftToBuild.erase( NextFile );
			BuildNextFile( rCompiler );
		}
	};

	CompileOptions Options;
	Options.IncludeDirs = m_IncludeDirectories;
	Options.Defines     = m_Defines;
	Options.Language    = File.extension() == ".c" ? CompileOptions::Language::C : CompileOptions::Language::CPlusPlus;
	Options.Action      = CompileOptions::Action::CompileAndAssemble;
	Options.InputFile   = File;
	Options.OutputFile  = m_Location / File.filename();
	Options.OutputFile.replace_extension( ".obj" );

	// Compile the file
	rCompiler.Compile( Options );

} // BuildNextFile

//////////////////////////////////////////////////////////////////////////

void Project::Link( ICompiler& rCompiler )
{
	rCompiler.Events.FinishedLinking += [ this ]( ICompiler& /*rCompiler*/, LinkOptions Options, int ExitCode )
	{
		Events.BuildFinished( *this, Options.OutputFile, ExitCode == 0 );
	};

	LinkOptions Options;
	Options.ObjectFiles        = std::move( m_FilesToLink );
	Options.LibraryDirectories = m_LibraryDirectories;
	Options.Libraries          = m_Libraries;
	Options.OutputFile         = m_Location / m_Name;
	Options.OutputType         = static_cast< uint32_t >( m_Kind );

	rCompiler.Link( Options );

} // Link
