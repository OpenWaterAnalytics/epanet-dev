[![Build Status](https://api.travis-ci.org/OpenWaterAnalytics/epanet-dev.svg)](https://travis-ci.org/OpenWaterAnalytics/epanet-dev)


# epanet-dev

This is a collaborative project to develop a new version of the EPANET computational engine for analyzing water distribution systems.

## Introduction

This project seeks to develop a new version of the EPANET computational engine and its associated API that includes recent advancements and improvements in water distribution system modeling. It is currently using EPANET 3 as its working title. Written in C++, it employs an object oriented approach that allows the code to be more modular, extensible, and easier to maintain.

EPANET was originally developed by the U.S. Environmental Protection Agency (USEPA) and placed in the public domain. The latest official version (2.2) can be found [here](https://www.epa.gov/water-research/epanet). The new version being developed by this project represents an independent effort that is part of the [Open Source EPANET Initiative](http://community.wateranalytics.org/t/announcement-of-an-open-source-epanet-initiative/117) and is neither supported nor endorsed by USEPA.

## Building EPANET 3

The source code can be compiled as both a shared library and a command-line executable. Any C++ compiler that supports the C++11 language standard can be used.

To build using CMake on Linux/Mac:
```
mkdir build && cd build
cmake .. && make
```
The shared library (`libepanet3.so`) will be found in the `/lib` sub-directory and the command-line executable (`run-epanet3`) will be in the `/bin` sub-directory.

To build using CMake on Windows with Visual Studio:
```
mkdir build && cd build
cmake -G "Visual Studio n yyyy" ..
cmake --build . --config Release
```
where `n yyyy` is the version and year of the Visual Studio release to use (e.g., 16 2019). Both the shared library (`epanet3.dll`) and the command-line executable (`run-epanet3.exe`) will be found in the `\bin\Release` sub-directory as will an `epanet3.lib` file needed to build applications that link to the library. 

To build using CMake on Windows with MinGW:
```
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```
Both the shared library (`libepanet3.dll`) and the command-line executable (`run-epanet3.exe`) will be found in the `\bin` sub-directory.

## Running EPANET 3
To run the command line executable under Linux/Mac enter the following command from a terminal window:
```
./run-epanet3 input.inp report.txt
```
where `input.inp` is the name of a properly formatted EPANET input file and `report.txt` is the name of a plain text file where results will be written. For Windows  , enter the following command in a Command Prompt window:
```
run-epanet3 input.inp report.txt
```
The EPANET 3 shared library contains an API that allows one to write custom applications by making function calls to it. A small example application can be found [here](https://github.com/OpenWaterAnalytics/epanet-dev/blob/develop/doc/Differences%20From%20EPANET2.md).

## API Reference

The EPANET 3 API has a similar flavor to that of EPANET 2, but all of the functions have been re-named and require that an EPANET project first be created and included as an argument in all function calls. (This makes the API capable of analyzing several projects in parallel in a thread safe manner.) EPANET 3 is able to read EPANET 2 input files but uses a different layout for its binary results file. Thus it will not be compatible with the current EPANET 2 GUI. Details of the API, including the changes and additions made to various computational components of EPANET, can be found in the 'docs' section of this repository.

You can access the full documentation at [wateranalytics.org/epanet-dev](http://wateranalytics.org/epanet-dev).

## Disclaimer
This project is still in its early developmental stage. Its robustness and the accuracy of its numerical results have not been thoroughly tested. Therefore it should not yet be used as a replacement EPANET 2 nor be used in any production code for specialized applications.

## License

The new version of EPANET will be distributed under the MIT license as described in the LICENSE file of this repository.
