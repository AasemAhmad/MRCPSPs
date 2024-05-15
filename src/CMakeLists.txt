# Dependencies

## loguru
find_package(loguru REQUIRED)
include_directories(${LOGURU_INCLUDE_DIR})

## pthread (for loguru)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#include_directories(${Python3_INCLUDE_DIRS})

add_subdirectory("External")
add_subdirectory("ProblemInstance")
add_subdirectory("ILPOptimizationModel")
add_subdirectory("InstanceReader")
add_subdirectory("ResultWriter")
add_subdirectory("Solution")
add_subdirectory("InstanceGenerator")

file (GLOB SOURCES "*.cpp")
add_executable (${PROJECT_NAME} ${SOURCES})



target_link_libraries(${PROJECT_NAME} PRIVATE External)

target_link_libraries(${PROJECT_NAME} PRIVATE ProblemInstance)
target_link_libraries(${PROJECT_NAME} PRIVATE CplexSolver)
target_link_libraries(${PROJECT_NAME} PRIVATE GurobiSolver)
target_link_libraries(${PROJECT_NAME} PRIVATE ILPOptimizationModel)
target_link_libraries(${PROJECT_NAME} PRIVATE InstanceReader)
target_link_libraries(${PROJECT_NAME} PRIVATE InstanceGenerator)
target_link_libraries(${PROJECT_NAME} PRIVATE ResultWriter)
target_link_libraries(${PROJECT_NAME} PRIVATE Solution)
target_link_libraries(${PROJECT_NAME} PRIVATE 
                      ${LOGURU_LIBRARIES}
                      Threads::Threads)



