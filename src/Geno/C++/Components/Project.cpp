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

#include <iostream>

//////////////////////////////////////////////////////////////////////////

Project::Project( std::filesystem::path Location )
	: m_Location( std::move( Location ) )
	, m_Name    ( "MyProject" )
{
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
	m_Files              = std::move( rrOther.m_Files );
	m_IncludeDirectories = std::move( rrOther.m_IncludeDirectories );
	m_Defines            = std::move( rrOther.m_Defines );
	m_Libraries          = std::move( rrOther.m_Libraries );
	m_Configurations     = std::move( rrOther.m_Configurations );
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

	if( m_Files.empty() )
		return;

	for( const std::filesystem::path& rFile : m_Files )
	{
		std::filesystem::path Extension = rFile.extension();

		// #TODO: Compiler will be per-file so this check is only temporary
		if( Extension == ".cpp"
		 || Extension == ".cxx"
		 || Extension == ".cc"
		 || Extension == ".c" )
		{
			m_FilesLeftToBuild.push_back( rFile );
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

	// Files
	if( !m_Files.empty() )
	{
		GCL::Object Files( "Files", std::in_place_type< GCL::Object::TableType > );

		for( const std::filesystem::path& rFile : m_Files )
		{
			const std::filesystem::path RelativePath = rFile.lexically_relative( m_Location );

			Files.AddChild( GCL::Object( RelativePath.string() ) );
		}

		Serializer.WriteObject( Files );
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

		for( const std::filesystem::path& rLibrary : m_Libraries )
		{
			const std::filesystem::path RelativePath = rLibrary.lexically_relative( m_Location );

			Libraries.AddChild( GCL::Object( RelativePath.string() ) );
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

	return true;

} // Deserialize

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

		if(      rKindString == "Application"    ) { pSelf->m_Kind = Kind::Application; }
		else if( rKindString == "StaticLibrary"  ) { pSelf->m_Kind = Kind::StaticLibrary; }
		else if( rKindString == "DynamicLibrary" ) { pSelf->m_Kind = Kind::DynamicLibrary; }
		else                                       { pSelf->m_Kind = Kind::Unspecified; }
	}
	else if( Name == "Files" )
	{
		for( const GCL::Object& rFilePathObj : Object.Table() )
		{
			std::filesystem::path FilePath = rFilePathObj.String();

			if( !FilePath.is_absolute() )
				FilePath = pSelf->m_Location / FilePath;

			FilePath = FilePath.lexically_normal();
			pSelf->m_Files.emplace_back( std::move( FilePath ) );
		}
	}
	else if( Name == "IncludeDirs" )
	{
		for( const GCL::Object& rFilePathObj : Object.Table() )
		{
			std::filesystem::path FilePath = rFilePathObj.String();

			if( !FilePath.is_absolute() )
				FilePath = pSelf->m_Location / FilePath;

			FilePath = FilePath.lexically_normal();
			pSelf->m_IncludeDirectories.emplace_back( std::move( FilePath ) );
		}
	}
	else if( Name == "Defines" )
	{
		for( const GCL::Object& rDefineObj : Object.Table() )
		{
			std::string Define = rDefineObj.String();

			pSelf->m_Defines.emplace_back( std::move( Define ) );
		}
	}
	else if( Name == "Libraries" )
	{
		for( const GCL::Object& rLibraryObj : Object.Table() )
		{
			std::filesystem::path LibraryPath = rLibraryObj.String();

			if( !LibraryPath.is_absolute() )
				LibraryPath = pSelf->m_Location / LibraryPath;

			LibraryPath = LibraryPath.lexically_normal();
			pSelf->m_Libraries.emplace_back( std::move( LibraryPath ) );
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

	auto File = std::find( m_Files.begin(), m_Files.end(), m_FilesLeftToBuild.back() );
	if( File == m_Files.end() )
	{
		// If the file was not found, remove it from the queue and try again
		m_FilesLeftToBuild.pop_back();
		BuildNextFile( rCompiler );
	}
	else
	{
		// Listen to every file compilation to check if we're 
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
		Options.Language    = File->extension() == ".c" ? CompileOptions::Language::C : CompileOptions::Language::CPlusPlus;
		Options.Action      = CompileOptions::Action::CompileAndAssemble;
		Options.InputFile   = *File;
		Options.OutputFile  = m_Location / File->filename();
		Options.OutputFile.replace_extension( ".obj" );

		// Compile the file
		rCompiler.Compile( Options );
	}

} // BuildNextFile

//////////////////////////////////////////////////////////////////////////

void Project::Link( ICompiler& rCompiler )
{
	rCompiler.Events.FinishedLinking += [ this ]( ICompiler& /*rCompiler*/, LinkOptions Options, int ExitCode )
	{
		Events.BuildFinished( *this, Options.OutputFile, ExitCode == 0 );
	};

	LinkOptions Options;
	Options.ObjectFiles     = std::move( m_FilesToLink );
	Options.LinkedLibraries = m_Libraries;
	Options.OutputFile      = m_Location / m_Name;
	Options.Kind            = m_Kind;

	rCompiler.Link( Options );

} // Link
