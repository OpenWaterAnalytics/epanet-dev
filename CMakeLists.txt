cmake_minimum_required (VERSION 2.8)

project ("EPANET")
set_property( GLOBAL PROPERTY USE_FOLDERS ON)
SET (CMAKE_C_FLAGS "-std=c99")
if(MSVC)
	# /Wall generated MANY warnings.  Defaulting to level 4, excluding C4100 (unreferenced formal parameter)
	# and C4571 (structured exceptions no longer caught)
	SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
	SET (CMAKE_CXX_FLAGS "/EHsc /W4 /wd4100 /wd4571")
        add_definitions(-D_CRT_SECURE_NO_DEPRECATE)  
else()
	# -I. is needed so gcc can find the generated export header
	SET (CMAKE_CXX_FLAGS "-Wall -std=c++11")
endif(MSVC)

add_subdirectory (src) 
enable_testing()
add_subdirectory (test)

