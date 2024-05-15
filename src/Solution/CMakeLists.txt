find_package(Python REQUIRED COMPONENTS Interpreter Development)

file(GLOB SOURCES "*.cpp")
add_library(Solution ${LIBRARY_LINKAGE} ${SOURCES})

target_link_libraries(Solution PRIVATE ResultWriter)
target_link_libraries(Solution  PUBLIC Python::Python)

