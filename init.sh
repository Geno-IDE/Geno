#!/bin/bash

PREMAKE_VERSION="5.0.0-alpha14"
PREMAKE_LOCATION="."
os=$(uname -s)
arch=$(uname -m)

# Linux system
if [ "$os" == "Linux" ]; then
	os="linux"

# Max OS X system
elif [ "$os" == "Darwin" ]; then
	os="osx"

# Assume Windows
else
	os="windows"
fi


# Initialize submodules
git submodule update --init
git submodule foreach --recursive "git submodule update --init"


# Windows setup
if [ "$os" == "windows" ]; then
	# Download premake executable
	curl -Lo "$PREMAKE_LOCATION/premake5.zip" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-windows.zip"
	unzip -oqu "$PREMAKE_LOCATION/premake5.zip" -d "$PREMAKE_LOCATION/"
	rm -f "$PREMAKE_LOCATION/premake5.zip"

# Linux setup
elif [ "$os" == "linux" ]; then
	# Determine whether we need to build from source or not
	if [ "$arch" == "x86_64" ]; then
		# Download premake executable
		curl -Lo "$PREMAKE_LOCATION/premake5.tar.gz" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-linux.tar.gz"
		tar -xvzf "$PREMAKE_LOCATION/premake5.tar.gz" -C "$PREMAKE_LOCATION/"
		rm -f "$PREMAKE_LOCATION/premake5.tar.gz"
	else
		# Download premake source package
		curl -Lo "$PREMAKE_LOCATION/premake5-src.zip" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-src.zip"
		unzip -o "$PREMAKE_LOCATION/premake5-src.zip" -d "$PREMAKE_LOCATION/"

		# Build premake
		echo "Building premake from source.."
		make -C "$PREMAKE_LOCATION/premake-$PREMAKE_VERSION/build/gmake.unix/"
		cp "$PREMAKE_LOCATION/premake-$PREMAKE_VERSION/bin/release/premake5" "$PREMAKE_LOCATION/"
		rm -rf "$PREMAKE_LOCATION/premake-$PREMAKE_VERSION/"
		rm -f "$PREMAKE_LOCATION/premake5-src.zip"
	fi

# Mac OS X setup
elif [ "$os" == "osx" ]; then
	# Download premake executable
	curl -Lo "$PREMAKE_LOCATION/premake5.tar.gz" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-macosx.tar.gz"
	tar -xvzf "$PREMAKE_LOCATION/premake5.tar.gz" -C "$PREMAKE_LOCATION/"
	rm -f "$PREMAKE_LOCATION/premake5.tar.gz"
fi
