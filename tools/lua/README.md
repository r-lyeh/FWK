# premake-ninja

[Premake](https://github.com/premake/premake-core) extension to support [Ninja](https://github.com/martine/ninja), because it's awesome. [![Build Status](https://travis-ci.org/jimon/premake-ninja.svg?branch=master)](https://travis-ci.org/jimon/premake-ninja) [![Build status](https://ci.appveyor.com/api/projects/status/3d2vot6y72nfo24y/branch/master?svg=true)](https://ci.appveyor.com/project/jimon/premake-ninja/branch/master)

### Implementation

For each project - configuration pair we create separate .ninja file. For solution we create build.ninja file which imports other .ninja files with subninja command.

Build.ninja file sets phony targets for configuration names so you can build them from command line. And default target is a first configuration name in your project (usually default).  

### TODO

- Resources are not supported
- Makefile not supported
- Bundles of any sort are not supported
- Clear methods are not supported
- C# not supported 
- D not supported

### Tested on

- msvc / win
	- ConsoleApp works
	- WindowedApp works 
	- StaticLib works
	- SharedLib works
	- win64 not tested
- clang / win
	- not tested
- clang / linux
	- not tested
- clang / osx
	- ConsoleApp works
	- WindowedApp works 
	- StaticLib works
	- SharedLib works
	- x86_64 tested, x86 not tested
- gcc / win
	- ConsoleApp works
	- WindowedApp works 
	- StaticLib works
	- SharedLib works
	- x86_64 tested, x86 not tested
- gcc / linux (tested on Ubuntu 15.04)
	- ConsoleApp works
	- WindowedApp works 
	- StaticLib works
	- SharedLib works
	- PCH works
	- Custom build rules works
	- x86_64 tested, x86 not tested

### Extra Tests

Part of integration tests of several generators in https://github.com/Jarod42/premake-sample-projects
