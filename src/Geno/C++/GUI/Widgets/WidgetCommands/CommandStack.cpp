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

void CommandStack::DoCommand( ICommand* pCommand )
{
	pCommand->Execute();
	m_Commands.emplace( pCommand );

} // DoCommand

//////////////////////////////////////////////////////////////////////////

void CommandStack::UndoCommand( CommandStack& rRedoCommandStack )
{
	if( !m_Commands.empty() )
	{
		rRedoCommandStack.m_Commands.emplace( m_Commands.top()->Undo() );
		m_Commands.pop();
	}

} // UndoCommand

//////////////////////////////////////////////////////////////////////////

void CommandStack::RedoCommand( CommandStack& rUndoCommandStack )
{
	if( !m_Commands.empty() )
	{
		rUndoCommandStack.m_Commands.emplace( m_Commands.top()->Redo() );
		m_Commands.pop();
	}

} // RedoCommand
