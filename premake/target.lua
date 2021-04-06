
require 'customizations'
require 'options'

-- Set system to iOS if the "ios" option was specified
if _OPTIONS[ 'ios' ] then
	_TARGET_OS = 'ios'
end
