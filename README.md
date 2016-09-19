[![Build Status](https://api.travis-ci.org/OpenWaterAnalytics/epanet-dev.svg)](https://travis-ci.org/OpenWaterAnalytics/epanet-dev)


# epanet-dev

This is a collaborative project to develop a new version of the EPANET computational engine for analyzing water distribution systems.

## Introduction

The last major update to EPANET, version 2.0, was made in 2000. Since that time many new advances in water distribution system modeling and suggestions for improving EPANET have been made. This project seeks to develop a new version of the EPANET computational engine and its associated API that includes many of these advances and improvements. It is currently using EPANET 3 as its working title. Written in C++, it employs an object oriented approach that allows the code to be more modular, extensible, and easier to maintain.

EPANET 2 was originally developed by the U.S. Environmental Protection Agency (USEPA) and placed in the public domain. The new version being developed by this project represents an independent effort that is part of the [Open Source EPANET Initiative](http://community.wateranalytics.org/t/announcement-of-an-open-source-epanet-initiative/117) and is neither supported nor endorsed by USEPA.

## Building & Installation

The source code can be compiled as a static or shared library, and a command-line application is included as well. Any C++ compiler that supports the C++11 language standard can be used.

To build using CMake on Linux/Mac:

```
mkdir build && cd build
cmake .. && make
cd bin
./run-epanet3 input.inp report.txt
```
### Windows & Visual Studio 
To build using CMake on Windows, make sure that CMake is installed and
accessible from the system PATH. Typing `cmake` at a command prompt should
result in a message from the program and not an error like " 'cmake' is not
recognized ...".  By default, CMake uses the latest version of 
[Visual Studio](https://www.visualstudio.com/en-us/visual-studio-homepage-vs.aspx)
to build projects on Windows.  Visual Studio 2015 does not install C++ 
development tools by default. Instructions for adding them are 
[here](https://blogs.msdn.microsoft.com/vcblog/2015/07/24/setup-changes-in-visual-studio-2015-affecting-c-developers/). 
To build the project, open a developer command prompt for VS 2015, navigate 
to the project root directory, and follow the commands:

```
mkdir build && cd build
cmake .. 
msbuild /p:Configuration=Release ALL_BUILD.vcxproj
cd bin\Release
run-epanet3 input.inp report.txt
```

### Windows & MSYS
To build using CMake on Windows, make sure that the programs CMake, make and the g++ 
C++ compiler are installed and accessible from your system PATH.
The compiler is available as part of MinGW which can be installed with
[MSYS](http://www.mingw.org/wiki/msys).
Typing `cmake`, `make`, and `g++` at a command prompt should result in a message from
each program rather than an error like " 'make' is not recognized ...".
Building on windows then follows almost the same steps as for Linux/Mac: 

```
mkdir build && cd build
cmake -G "MSYS Makefiles" .. && make
cd bin
run-epanet3 input.inp report.txt
```


## API Reference

The EPANET 3 API has a similar flavor to version 2, but all of the functions have been re-named and require that an EPANET project first be created and included as an argument in all function calls. (This makes the API capable of analyzing several projects in parallel in a thread safe manner.) EPANET 3 is able to read EPANET 2 input files but uses a different layout for its binary results file. Thus it will not be compatible with the current EPANET 2 GUI. Details of the API, including the changes and additions made to various computational components of EPANET, can be found in the 'docs' section of this repository.

You can access the full documentation at [wateranalytics.org/epanet-dev](http://wateranalytics.org/epanet-dev).

## License

The new version of EPANET will be distributed under the MIT license as described in the LICENSE file of this repository.
