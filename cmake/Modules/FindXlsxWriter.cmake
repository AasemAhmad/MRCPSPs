# Locate XlsxWriter headers and libraries

if(NOT XlsxWriter_INCLUDE_DIR)
    find_path(XlsxWriter_INCLUDE_DIR
        NAMES xlsxwriter.h
        PATH_SUFFIXES include
    )
endif()

if(NOT XlsxWriter_LIBRARY)
    find_library(XlsxWriter_LIBRARY
        NAMES xlsxwriter
        PATH_SUFFIXES lib lib64
    )
endif()

# Check if XlsxWriter was found
if(XlsxWriter_INCLUDE_DIR AND XlsxWriter_LIBRARY)
    set(XlsxWriter_FOUND TRUE)
else()
    set(XlsxWriter_FOUND FALSE)
endif()

# Provide some user-friendly messages
if(XlsxWriter_FOUND)
    message(STATUS "Found XlsxWriter: ${XlsxWriter_INCLUDE_DIR}, ${XlsxWriter_LIBRARY}")
else()
    message(STATUS "XlsxWriter not found")
endif()

# Provide the XlsxWriter_FOUND and other variables to the calling code
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XlsxWriter DEFAULT_MSG XlsxWriter_INCLUDE_DIR XlsxWriter_LIBRARY)
