cmake_minimum_required (VERSION 2.8)

project ("EPANET")
set_property( GLOBAL PROPERTY USE_FOLDERS ON)
SET (CMAKE_C_FLAGS "-std=c99")
if(MSVC)
	# /Wall generated MANY warnings.  Defaulting to level 4, excluding C4100 (unreferenced formal parameter)
	# and enabling C4571, C4263, and C4264 
	SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
	SET (CMAKE_CXX_FLAGS "/EHsc /W4 /wd4100 /w44571 /w44263 /w44264")
else()
	# -I. is needed so gcc can find the generated export header
	SET (CMAKE_CXX_FLAGS "-Wall -std=c++11")
endif(MSVC)

add_subdirectory (src) 
enable_testing()
add_subdirectory (test)

