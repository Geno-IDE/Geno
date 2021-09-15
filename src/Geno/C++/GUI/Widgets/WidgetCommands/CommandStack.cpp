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

#include "CommandStack.h"

CommandStack::~CommandStack( void )
{
	for( size_t i = 0; i < m_pCommands.size(); ++i )
	{
		auto& rCommand = m_pCommands.top();
		if( rCommand )
		{
			delete rCommand;
			rCommand = nullptr;
		}
	}

} // ~CommandStack

//////////////////////////////////////////////////////////////////////////

void CommandStack::DoCommand( ICommand* pCommand )
{
	pCommand->Execute();
	m_pCommands.push( pCommand );
} // DoCommand

//////////////////////////////////////////////////////////////////////////

void CommandStack::UndoCommand( CommandStack& rRedoCommandStack )
{
	if( !m_pCommands.empty() )
	{
		ICommand*& rpCommand = m_pCommands.top();

		rRedoCommandStack.m_pCommands.push( rpCommand->Undo() );

		delete rpCommand;
		rpCommand = nullptr;

		m_pCommands.pop();
	}

} // UndoCommand

//////////////////////////////////////////////////////////////////////////

void CommandStack::RedoCommand( CommandStack& rUndoCommandStack )
{
	if( !m_pCommands.empty() )
	{
		ICommand*& rpCommand = m_pCommands.top();

		rUndoCommandStack.m_pCommands.push( rpCommand->Redo() );

		delete rpCommand;
		rpCommand = nullptr;

		m_pCommands.pop();
	}

} // RedoCommand
