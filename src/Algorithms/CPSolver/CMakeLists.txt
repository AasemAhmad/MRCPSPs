set(CPLEX_LIBS ${CPLEX_LIBRARIES} ${CPLEX_ILOCPLEX_LIBRARIES} ${CPLEX_CONCERT_LIBRARIES} ${CPLEX_CP_LIBRARIES})
add_definitions(${CPLEX_ILOCPLEX_DEFINITIONS} ${CPLEX_CONCERT_DEFINITIONS} -Wno-unused-private-field)

file(GLOB SOURCES "*.cpp")
add_library(CPSolver ${LIBRARY_LINKAGE} ${SOURCES})

target_link_libraries(CPSolver PUBLIC ${CPLEX_LIBS})
target_link_libraries(CPSolver PUBLIC dl)
target_link_libraries(CPSolver PRIVATE Solution)
target_link_libraries(CPSolver PRIVATE Shared)
