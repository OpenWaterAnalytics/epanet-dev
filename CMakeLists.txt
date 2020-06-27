cmake_minimum_required (VERSION 2.8)
SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

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

SET (epanet_lib_sources 
src/Core/datamanager.cpp
src/Core/diagnostics.cpp
src/Core/epanet3.cpp
src/Core/error.cpp
src/Core/hydbalance.cpp
src/Core/hydengine.cpp
src/Core/network.cpp
src/Core/options.cpp
src/Core/project.cpp
src/Core/qualbalance.cpp
src/Core/qualengine.cpp
src/Core/units.cpp
src/Elements/control.cpp
src/Elements/curve.cpp
src/Elements/demand.cpp
src/Elements/element.cpp
src/Elements/emitter.cpp
src/Elements/junction.cpp
src/Elements/link.cpp
src/Elements/node.cpp
src/Elements/pattern.cpp
src/Elements/pipe.cpp
src/Elements/pump.cpp
src/Elements/pumpcurve.cpp
src/Elements/qualsource.cpp
src/Elements/reservoir.cpp
src/Elements/tank.cpp
src/Elements/valve.cpp
src/Input/controlparser.cpp
src/Input/curveparser.cpp
src/Input/inputparser.cpp
src/Input/inputreader.cpp
src/Input/linkparser.cpp
src/Input/nodeparser.cpp
src/Input/optionparser.cpp
src/Input/patternparser.cpp
src/Models/demandmodel.cpp
src/Models/headlossmodel.cpp
src/Models/leakagemodel.cpp
src/Models/pumpenergy.cpp
src/Models/qualmodel.cpp
src/Models/tankmixmodel.cpp
src/Output/outputfile.cpp
src/Output/projectwriter.cpp
src/Output/reportfields.cpp
src/Output/reportwriter.cpp
src/Solvers/ggasolver.cpp
src/Solvers/hydsolver.cpp
src/Solvers/ltdsolver.cpp
src/Solvers/matrixsolver.cpp
src/Solvers/qualsolver.cpp
src/Solvers/sparspak.cpp
src/Solvers/sparspaksolver.cpp
src/Utilities/graph.cpp
src/Utilities/mempool.cpp
src/Utilities/segpool.cpp
src/Utilities/utilities.cpp
)

SET (epanet_lib_headers 
src/Core/constants.h
src/Core/datamanager.h
src/Core/diagnostics.h
src/Core/error.h
src/Core/hydbalance.h
src/Core/hydengine.h
src/Core/network.h
src/Core/options.h
src/Core/project.h
src/Core/qualbalance.h
src/Core/qualengine.h
src/Core/units.h
src/Elements/control.h
src/Elements/curve.h
src/Elements/demand.h
src/Elements/element.h
src/Elements/emitter.h
src/Elements/junction.h
src/Elements/link.h
src/Elements/node.h
src/Elements/pattern.h
src/Elements/pipe.h
src/Elements/pump.h
src/Elements/pumpcurve.h
src/Elements/qualsource.h
src/Elements/reservoir.h
src/Elements/tank.h
src/Elements/valve.h
src/Input/controlparser.h
src/Input/curveparser.h
src/Input/inputparser.h
src/Input/inputreader.h
src/Input/linkparser.h
src/Input/nodeparser.h
src/Input/optionparser.h
src/Input/patternparser.h
src/Models/demandmodel.h
src/Models/headlossmodel.h
src/Models/leakagemodel.h
src/Models/pumpenergy.h
src/Models/qualmodel.h
src/Models/tankmixmodel.h
src/Output/outputfile.h
src/Output/projectwriter.h
src/Output/reportfields.h
src/Output/reportwriter.h
src/Solvers/ggasolver.h
src/Solvers/hydsolver.h
src/Solvers/ltdsolver.h
src/Solvers/matrixsolver.h
src/Solvers/qualsolver.h
src/Solvers/sparspak.h
src/Solvers/sparspaksolver.h
src/Utilities/graph.h
src/Utilities/mempool.h
src/Utilities/segpool.h
src/Utilities/utilities.h
)

source_group("Core" REGULAR_EXPRESSION "src/Core/.*")
source_group("Elements" REGULAR_EXPRESSION "src/Elements/.*")
source_group("Input" REGULAR_EXPRESSION "src/Input/.*")
source_group("Models" REGULAR_EXPRESSION "src/Models/.*")
source_group("Output" REGULAR_EXPRESSION "src/Output/.*")
source_group("Solvers" REGULAR_EXPRESSION "src/Solvers/.*")
source_group("Utilities" REGULAR_EXPRESSION "src/Utilities/.*")

include_directories(src src/Core src/Elements src/Input src/Output src/Utilities src/Solvers)

add_library(epanet3 SHARED ${epanet_lib_sources} ${epanet_lib_headers})

add_executable(run-epanet3 src/CLI/main.cpp)
target_link_libraries(run-epanet3 LINK_PUBLIC epanet3)