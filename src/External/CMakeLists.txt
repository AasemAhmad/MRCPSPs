add_subdirectory(Cplex)
add_subdirectory(Gurobi)
add_subdirectory(ILPSolverModel)

file(GLOB SOURCES "*.cpp")
add_library(External ${LIBRARY_LINKAGE} ${SOURCES})