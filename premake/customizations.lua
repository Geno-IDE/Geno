
-- Set default actions so that we can run premake directly and without arguments
if( not _ACTION ) then
	local host = os.host()
	local specialActions = {
		windows = 'vs2019',
		macosx  = 'xcode4',
	}
	_ACTION = specialActions[ host ] or 'gmake2'
end
