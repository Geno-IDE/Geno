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

#include "Workspace.h"

#include "Auxiliary/jsonSerializer.h"
#include "Compilers/CompilerGCC.h"
#include "Compilers/CompilerMSVC.h"
#include "GUI/Widgets/StatusBar.h"

#include "Auxiliary/JSONSerializer.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <Common/Async/JobSystem.h>

//////////////////////////////////////////////////////////////////////////

Workspace::Workspace( std::string Name, std::filesystem::path Location )
	: INode( std::move( Location ), std::move( Name ), NodeKind::Workspace )
{
	AddChild( new Group( m_Location, "", true ) ); // Create Default Group

} // Workspace

//////////////////////////////////////////////////////////////////////////

void Workspace::Build( void )
{
	if( !m_pChildren.empty() )
	{
		UTF8Converter                            UTF8Converter;
		std::vector< JobSystem::JobPtr >         LinkerJobs;
		std::vector< std::string >               LinkerJobProjectNames;
		std::shared_ptr< std::filesystem::path > LinkerOutput = std::make_shared< std::filesystem::path >();

		// Sort projects so that the link jobs exist to be depended upon
		std::vector< std::reference_wrapper< Project > > ProjectRefs;
		std::copy( m_Projects.begin(), m_Projects.end(), std::back_inserter( ProjectRefs ) );
		std::sort( ProjectRefs.begin(), ProjectRefs.end(), []( const Project& rA, const Project& rB )
			{
				auto& rLibraries = rB.m_LocalConfiguration.m_Libraries;
				return std::find( rLibraries.begin(), rLibraries.end(), rA.m_Name ) != rLibraries.end();
			}
		);

		for( Project& rProject : ProjectRefs )
		{
			Configuration                                           Configuration = m_BuildMatrix.CurrentConfiguration();
			std::vector< JobSystem::JobPtr >                        LinkerDependencies;
			std::vector< std::shared_ptr< std::filesystem::path > > CompilerOutputs;

			Configuration.Override( rProject.m_LocalConfiguration );

			if( !Configuration.m_OutputDir )
				Configuration.m_OutputDir = rProject.m_Location;

			// TODO: Remove any duplicate files, since a single file can exist in multiple filters

			for( const FileFilter& rFileFilter : rProject.m_FileFilters )
			{
				for( const std::filesystem::path& rFile : rFileFilter.Files )
				{
					auto Extension = rFile.extension();

					// Skip any files that shouldn't be compiled
					// TODO: We want to support other languages in the future. Perhaps store the compiler in each file-config?
					if( Extension != ".c"
					 && Extension != ".cc"
					 && Extension != ".cpp"
					 && Extension != ".cxx"
					 && Extension != ".c++" )
						continue;

					auto Output = std::make_shared< std::filesystem::path >();

					CompilerOutputs.push_back( Output );

					LinkerDependencies.push_back( JobSystem::Instance().NewJob(
						[ Configuration, rFile, Output ]( void )
						{
							if( !Configuration.m_Compiler )
							{
								std::cerr << "Failed to compile " << rFile << ". No compiler active!\n";
								return;
							}

							if( auto Result = Configuration.m_Compiler->Compile( Configuration, rFile ) )
							{
								*Output = *Result;
							}
						}
					) );
				}
			}

			// Assemble a list of link jobs for projects that this depends on
			for( std::string& rLibrary : Configuration.m_Libraries )
			{
				auto Name = std::find( LinkerJobProjectNames.begin(), LinkerJobProjectNames.end(), rLibrary );
				if( Name != LinkerJobProjectNames.end() )
					LinkerDependencies.push_back( *std::next( LinkerJobs.begin(), std::distance( LinkerJobProjectNames.begin(), Name ) ) );
			}

			const std::wstring  ProjectName = UTF8Converter.from_bytes( rProject.m_Name );
			const Project::Kind Kind        = rProject.m_Kind;

			LinkerJobProjectNames.push_back( rProject.m_Name );
			LinkerJobs.push_back( JobSystem::Instance().NewJob(
				[ Configuration, ProjectName, Kind, CompilerOutputs, LinkerOutput ]( void )
				{
					std::vector< std::filesystem::path > InputFiles;

					for( auto& rInputFile : CompilerOutputs )
						if( !rInputFile->empty() )
							InputFiles.emplace_back( std::move( *rInputFile ) );

					if( !InputFiles.empty() )
					{
						if( auto Result = Configuration.m_Compiler->Link( Configuration, InputFiles, ProjectName, Kind ) )
							*LinkerOutput = *Result;
					}
				},
				LinkerDependencies
			) );
		}

		JobSystem::Instance().NewJob(
			[ this, LinkerJobs, LinkerOutput ]( void )
			{
				if( auto& rLinkerOutput = *LinkerOutput; !rLinkerOutput.empty() )
				{
					std::cout << "Done building workspace\n";

					Events.BuildFinished( *this, rLinkerOutput, true );
				}
				else
				{
					std::cout << "Failed to build workspace\n";

					Events.BuildFinished( *this, "", false );
				}
			},
			LinkerJobs
		);
	}

} // Build

//////////////////////////////////////////////////////////////////////////

bool Workspace::Serialize( void )
{
	if( m_Location.empty() )
		return false;

	JSONSerializer Serializer( ( m_Location / m_Name ).replace_extension( EXTENSION ) );

	// Matrix table
	{
		// TODO Serialize Build Matrix
		/*
		* Serializer.Object( "Matrix", [ this, &Serializer ]( void )
			{
				for( const BuildMatrix::Column& rColumn : m_BuildMatrix.m_Columns )
				{
					Serializer.Object( rColumn.Name, [ & ]( void ) {

						for( const auto& rConfiguration : rColumn.Configurations )
							{
								Serializer.Object( rConfiguration.first, [ & ]( void )
									{
										if(rConfiguration.second.m_Compiler || rConfiguration.second.m_Optimization)
										{
											if( rConfiguration.second.m_Compiler)
											{
												Serializer.Add( "Compiler", std::string( rConfiguration.second.m_Compiler->GetName() ) );
											}
										}

									} );
							}

					} );
				}
			} );
		*/
	}

	// Groups
	{
		Serializer.Object( "Groups", [ & ]()
			{
				for( INode* pNode : m_pChildren )
				{
					Group* pGroup = ( Group* )pNode;
					pGroup->Serialize( Serializer );
				}

			} );
	}

	return true;

} // Serialize

//////////////////////////////////////////////////////////////////////////

bool Workspace::Deserialize( void )
{
	if( m_Location.empty() )
		return false;

	rapidjson::Document Doc;

	std::ifstream     gwks( ( m_Location / m_Name ).replace_extension( EXTENSION ), std::ios::in );
	std::stringstream Content;
	Content << gwks.rdbuf();
	gwks.close();
	Doc.Parse( Content.str().c_str() );

	for( auto It = Doc.MemberBegin(); It < Doc.MemberEnd(); ++It )
	{
		const std::string MemberName = It->name.GetString();

		if( MemberName == "Matrix" )
		{
			//TODO Deserialize Build Matrix Column
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
					Group* pGroup = new Group( m_Location, GroupName, true );
					pGroup->Deserialize( GroupsIt );
					AddChild( std::move( pGroup ) );
				}
			}
		}
	}

	return true;

} // Deserialize

//////////////////////////////////////////////////////////////////////////

void Workspace::Rename( std::string Name )
{
	const std::filesystem::path OldPath = ( m_Location / m_Name ).replace_extension( EXTENSION );

	if( std::filesystem::exists( OldPath ) )
	{
		const std::filesystem::path NewPath = ( m_Location / Name ).replace_extension( EXTENSION );
		std::filesystem::rename( OldPath, NewPath );
	}

	m_Name = std::move( Name );
	Serialize();

} // Rename
