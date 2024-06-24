add_subdirectory(CPSolver)
add_subdirectory(ILPOptimizationModel)

add_library(Algorithms INTERFACE)

include_directories(${CPLEX_INCLUDE_DIRS})
target_link_libraries(Algorithms INTERFACE CPSolver)
target_link_libraries(Algorithms INTERFACE ILPOptimizationModel)

set(CPLEX_CP_INCLUDE_DIRS ${CPLEX_CP_INCLUDE_DIRS} CACHE INTERNAL "Include directories for CP")