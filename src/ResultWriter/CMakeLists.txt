find_package(XlsxWriter REQUIRED)

file(GLOB SOURCES "*.cpp")
add_library(ResultWriter ${LIBRARY_LINKAGE} ${SOURCES})

target_include_directories(ResultWriter PRIVATE ${XlsxWriter_INCLUDE_DIR})
target_link_libraries(ResultWriter PRIVATE ${XlsxWriter_LIBRARY})
