
require 'customizations'

utils = { }

-- The list of platforms differ depending on he specified action
function utils.get_platform()
	if os.ishost( 'windows' ) then
		return 'x64'
	else
		return os.outputof( 'uname -m' )
	end
end

-- Return an iterator for the allowed systems
function utils.each_system()
	local field = premake.field.get( 'system' )
	local i

	return function()
		i = next( field.allowed, i )
		return field.allowed[ i ]
	end
end
