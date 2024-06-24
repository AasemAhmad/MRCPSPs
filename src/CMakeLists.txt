find_package(loguru REQUIRED)
include_directories(${LOGURU_INCLUDE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(CPLEX REQUIRED)
include_directories(${CPLEX_INCLUDE_DIRS} ${CPLEX_ILOCPLEX_INCLUDE_DIRS} ${CPLEX_CONCERT_INCLUDE_DIRS} ${CPLEX_CP_INCLUDE_DIRS})
#set(CPLEX_LIBS ${CPLEX_LIBRARIES} ${CPLEX_ILOCPLEX_LIBRARIES} ${CPLEX_CONCERT_LIBRARIES} ${CPLEX_CP_LIBRARIES})
#add_definitions(${CPLEX_ILOCPLEX_DEFINITIONS} ${CPLEX_CONCERT_DEFINITIONS} -Wno-unused-private-field)



add_subdirectory("External")
add_subdirectory("ProblemInstance")
add_subdirectory("Algorithms")
add_subdirectory("InstanceReader")
add_subdirectory("ResultWriter")
add_subdirectory("Solution")
add_subdirectory("InstanceGenerator")
add_subdirectory("Shared")

file (GLOB SOURCES "*.cpp")
add_executable (${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE External)
target_link_libraries(${PROJECT_NAME} PRIVATE ProblemInstance)
target_link_libraries(${PROJECT_NAME} PRIVATE Algorithms)
target_link_libraries(${PROJECT_NAME} PRIVATE InstanceReader)
target_link_libraries(${PROJECT_NAME} PRIVATE InstanceGenerator)
target_link_libraries(${PROJECT_NAME} PRIVATE ResultWriter)
target_link_libraries(${PROJECT_NAME} PRIVATE Solution)
target_link_libraries(${PROJECT_NAME} PRIVATE Shared)
target_link_libraries(${PROJECT_NAME} PRIVATE 
                      ${LOGURU_LIBRARIES}
                      Threads::Threads)

