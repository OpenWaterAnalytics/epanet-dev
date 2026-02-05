# cmakelists.txt for epanet3 src 

SET (epanet_lib_sources 
Core/datamanager.cpp
Core/diagnostics.cpp
Core/epanet3.cpp
Core/error.cpp
Core/hydbalance.cpp
Core/hydengine.cpp
Core/network.cpp
Core/options.cpp
Core/project.cpp
Core/qualbalance.cpp
Core/qualengine.cpp
Core/units.cpp
Elements/control.cpp
Elements/curve.cpp
Elements/demand.cpp
Elements/element.cpp
Elements/emitter.cpp
Elements/junction.cpp
Elements/link.cpp
Elements/node.cpp
Elements/pattern.cpp
Elements/pipe.cpp
Elements/pump.cpp
Elements/pumpcurve.cpp
Elements/qualsource.cpp
Elements/reservoir.cpp
Elements/tank.cpp
Elements/valve.cpp
Input/controlparser.cpp
Input/curveparser.cpp
Input/inputparser.cpp
Input/inputreader.cpp
Input/linkparser.cpp
Input/nodeparser.cpp
Input/optionparser.cpp
Input/patternparser.cpp
Models/demandmodel.cpp
Models/headlossmodel.cpp
Models/leakagemodel.cpp
Models/pumpenergy.cpp
Models/qualmodel.cpp
Models/tankmixmodel.cpp
Output/outputfile.cpp
Output/projectwriter.cpp
Output/reportfields.cpp
Output/reportwriter.cpp
Solvers/ggasolver.cpp
Solvers/hydsolver.cpp
Solvers/ltdsolver.cpp
Solvers/matrixsolver.cpp
Solvers/qualsolver.cpp
Solvers/sparspak.cpp
Solvers/sparspaksolver.cpp
Utilities/graph.cpp
Utilities/mempool.cpp
Utilities/segpool.cpp
Utilities/utilities.cpp
)

SET (epanet_lib_headers 
Core/constants.h
Core/datamanager.h
Core/diagnostics.h
Core/error.h
Core/hydbalance.h
Core/hydengine.h
Core/network.h
Core/options.h
Core/project.h
Core/qualbalance.h
Core/qualengine.h
Core/units.h
Elements/control.h
Elements/curve.h
Elements/demand.h
Elements/element.h
Elements/emitter.h
Elements/junction.h
Elements/link.h
Elements/node.h
Elements/pattern.h
Elements/pipe.h
Elements/pump.h
Elements/pumpcurve.h
Elements/qualsource.h
Elements/reservoir.h
Elements/tank.h
Elements/valve.h
Input/controlparser.h
Input/curveparser.h
Input/inputparser.h
Input/inputreader.h
Input/linkparser.h
Input/nodeparser.h
Input/optionparser.h
Input/patternparser.h
Models/demandmodel.h
Models/headlossmodel.h
Models/leakagemodel.h
Models/pumpenergy.h
Models/qualmodel.h
Models/tankmixmodel.h
Output/outputfile.h
Output/projectwriter.h
Output/reportfields.h
Output/reportwriter.h
Solvers/ggasolver.h
Solvers/hydsolver.h
Solvers/ltdsolver.h
Solvers/matrixsolver.h
Solvers/qualsolver.h
Solvers/sparspak.h
Solvers/sparspaksolver.h
Utilities/graph.h
Utilities/mempool.h
Utilities/segpool.h
Utilities/utilities.h
)

source_group("Core" REGULAR_EXPRESSION "Core/.*")
source_group("Elements" REGULAR_EXPRESSION "Elements/.*")
source_group("Input" REGULAR_EXPRESSION "Input/.*")
source_group("Models" REGULAR_EXPRESSION "Models/.*")
source_group("Output" REGULAR_EXPRESSION "Output/.*")
source_group("Solvers" REGULAR_EXPRESSION "Solvers/.*")
source_group("Utilities" REGULAR_EXPRESSION "Utilities/.*")

include_directories(. Core Elements Input Output Utilities Solvers)

# static for ease of testing
add_library(epanet3 STATIC ${epanet_lib_sources} ${epanet_lib_headers})

add_executable(run-epanet3 CLI/main.cpp)
target_link_libraries(run-epanet3 LINK_PUBLIC epanet3)

# put the library and executable in bin
set_target_properties( epanet3 run-epanet3
                       PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin) 
