include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set various c/cpp flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")

SYSR_INCLUDE_DIR(qcril-nr)
SYSR_INCLUDE_DIR(qmi)
SYSR_INCLUDE_DIR(qmi-framework)

# Set target library name as libqcrilFramework
set(TARGET_LIBQCRIL_DB ril-db)

# Library source files
file(GLOB libqcrilDb_src "src/*.cpp")

# Build libqcrilFramework shared library
add_library (${TARGET_LIBQCRIL_DB} STATIC
    "${libqcrilDb_src}"
)

target_include_directories ( ${TARGET_LIBQCRIL_DB}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )
target_link_libraries(${TARGET_LIBQCRIL_DB}
    qtimutex
    qcrilNrLogger
    sqlite3
    qcrilNrUtils
)

install ( TARGETS ${TARGET_LIBQCRIL_DB}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )
