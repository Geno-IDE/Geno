require 'vstudio'

-- Set default actions so that we can run premake directly and without arguments
if( not _ACTION ) then
	local host = os.host()
	local specialActions = {
		windows = 'vs2019',
		macosx  = 'xcode4',
	}
	_ACTION = specialActions[ host ] or 'gmake2'
end

-- Add extra properties to custom build steps
premake.override( premake.vstudio.vc2010, 'linkObjects', function( base, fcfg, condition )
	base( fcfg, condition )

	-- Allow custom build steps to run in parallel
	premake.vstudio.vc2010.element( 'BuildInParallel', condition, 'true' )
end )
