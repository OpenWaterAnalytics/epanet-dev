cmake_minimum_required (VERSION 2.8)
SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

project (EPANET)
SET (CMAKE_C_FLAGS "-std=c99")
if(MSVC)
# /Wall generated MANY warnings.  Defaulting to level 4, excluding C4100 (unreferenced formal parameter)
# and enabling C4571, C4263, and C4264 
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET (CMAKE_CXX_FLAGS "/W4 /wd4100 /w44571 /w44263 /w44264")
else()
# -I. is needed so gcc can find the generated export header
SET (CMAKE_CXX_FLAGS "-Wall -std=c++11")
endif(MSVC)

SET (epanet_lib_sources src/Elements/junction.cpp
src/Input/patternparser.cpp
src/Core/datamanager.cpp	src/Elements/link.cpp		src/Models/demandmodel.cpp
src/Core/diagnostics.cpp	src/Elements/node.cpp		src/Models/headlossmodel.cpp
src/Core/epanet3.cpp		src/Elements/pattern.cpp	src/Models/leakagemodel.cpp
src/Core/error.cpp		src/Elements/pipe.cpp		src/Models/pumpenergy.cpp
src/Core/hydbalance.cpp		src/Elements/pump.cpp		src/Models/qualmodel.cpp
src/Core/hydengine.cpp		src/Elements/pumpcurve.cpp	src/Models/tankmixmodel.cpp
src/Core/network.cpp		src/Elements/qualsource.cpp	src/Output/outputfile.cpp
src/Core/options.cpp		src/Elements/reservoir.cpp	src/Output/projectwriter.cpp
src/Core/project.cpp		src/Elements/tank.cpp		src/Output/reportfields.cpp
src/Core/qualbalance.cpp	src/Elements/valve.cpp		src/Output/reportwriter.cpp
src/Core/qualengine.cpp		src/Input/controlparser.cpp	src/Utilities/graph.cpp
src/Core/units.cpp		src/Input/curveparser.cpp	src/Utilities/hashtable.cpp
src/Elements/control.cpp	src/Input/inputparser.cpp	src/Utilities/mempool.cpp
src/Elements/curve.cpp		src/Input/inputreader.cpp	src/Utilities/segpool.cpp
src/Elements/demand.cpp		src/Input/linkparser.cpp	src/Utilities/utilities.cpp
src/Elements/element.cpp	src/Input/nodeparser.cpp
src/Elements/emitter.cpp	src/Input/optionparser.cpp
src/Solvers/ggasolver.cpp	src/Solvers/ltdsolver.cpp	src/Solvers/qualsolver.cpp	src/Solvers/sparspaksolver.cpp
src/Solvers/hydsolver.cpp	src/Solvers/matrixsolver.cpp	src/Solvers/sparspak.cpp

)

include_directories(src src/Core src/Elements src/Input src/Output src/Utilities src/Solvers)

add_library(epanet3 SHARED ${epanet_lib_sources})

add_executable(run-epanet3 src/CLI/main.cpp)
target_link_libraries(run-epanet3 LINK_PUBLIC epanet3)
