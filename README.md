<p align="center">
	<a href="https://github.com/Geno-IDE/Geno/graphs/contributors"><img alt="Contributors" src="https://img.shields.io/github/contributors/Geno-IDE/Geno?style=for-the-badge"></a>
	<a href="/LICENSE"><img alt="License" src="https://img.shields.io/github/license/Geno-IDE/Geno?style=for-the-badge"></a>
	<a href="https://discord.gg/dUd5aDSBAN"><img alt="Discord" src="https://img.shields.io/discord/854043545240338442?color=%238A9CFC&label=%20Discord&logo=Discord&logoColor=%238A9CFC&style=for-the-badge"></a>
</p>

<p align="center">
	<a href="https://github.com/Geno-IDE/Geno/actions/workflows/ci-windows.yml"><img alt="GitHub Workflow Status" src="https://img.shields.io/github/workflow/status/Geno-IDE/Geno/CI%20(Windows)?label=Windows&logo=Windows&logoColor=FFF&style=for-the-badge"></a>
	<a href="https://github.com/Geno-IDE/Geno/actions/workflows/ci-linux.yml"><img alt="GitHub Workflow Status" src="https://img.shields.io/github/workflow/status/Geno-IDE/Geno/CI%20(Linux)?label=Linux&logo=Linux&logoColor=FFF&style=for-the-badge"></a>
	<a href="https://github.com/Geno-IDE/Geno/actions/workflows/ci-macos.yml"><img alt="GitHub Workflow Status" src="https://img.shields.io/github/workflow/status/Geno-IDE/Geno/CI%20(macOS)?label=macOS&logo=Apple&logoColor=FFF&style=for-the-badge"></a>
</p>
<br>

<p align="center">
	<img src="resources/icon.png" alt="Geno" width="80" height="80">
</p>
<h3 align="center">Geno</h3>
<p align="center">
	Native IDE centered around clean cross-compilation and distribution
	<br>
	<a href="https://github.com/Geno-IDE/Geno/wiki/User-Guide"><strong>User Guide »</strong></a>
	<br>
	<br>
	<br>
	<a href="https://github.com/Geno-IDE/Geno/releases">View Releases</a>
	·
	<a href="https://github.com/Geno-IDE/Geno/issues">Report a Bug</a>
	·
	<a href="https://github.com/Geno-IDE/Geno/fork">Fork Geno</a>
</p>
<hr noshade>

<h3 align="center">Building</h3>
<p align="center">
	This project uses <a href="https://premake.github.io/"><img src="https://raw.githubusercontent.com/premake/premake.github.io/4258dd78ca8a237b7f1405606aac341f6bcea727/img/premake-logo.png" width="18" height="18">Premake</a>.
	<br>
	<br>
	<a href="https://premake.github.io/docs/What-Is-Premake">What is Premake?</a>
	·
	<a href="https://premake.github.io/download">Download</a>
	·
	<a href="https://premake.github.io/docs/Using-Premake">Usage</a>
	<br>
	<br>
</p>

<h5 align="center">Submodules</h5>
<p align="center">
	All third-party libraries used are located in <a href="/third_party">/third_party/</a>.
	Some of them are stored as git submodules, which means that, unless your submodules are initialized automatically, you need to run: <code>git submodule update --init</code>
	<br>
	<br>
</p>

<h5 align="center">Generating the project files</h5>
<p align="center">
	To generate the project files, you can simply run the premake executable.
	<br>
	However, if premake is located somewhere else on your disk, or you want to use another generator, you can open a terminal in the project root directory and run it like so: <code>premake5 &lt;action&gt;</code> where &lt;action&gt; can be a generator of your choice, or left empty to use the system default
	<br>
	<br>
</p>

<h5 align="center">Linux Prerequisite Packages</h5>
<p align="center">
	<code>build-essential libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev libglu1-mesa-dev libncurses-dev</code>
	<br>
	<br>
</p>

<h5 align="center">Compiling</h5>
<p align="center">
	Finally, after generating the project files, the application is ready to be built using the corresponding IDE or build tool.
</p>
<hr noshade>

<h3 align="center">License</h3>
<p align="center">
	Distributed under the zlib/libpng License. See <a href="LICENSE">LICENSE</a> for more information.
</p>
