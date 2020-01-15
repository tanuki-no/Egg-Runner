# Runner library test

# Define includes
INCLUDE_DIRECTORIES (
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_INSTALL_FULL_INCLUDEDIR}
  )

# Tests
# -----------------------------------------------------------------
SET (
  TEST

  "t01"
  "t02"
  "t03"
  )

# Library test
# -----------------------------------------------------------------
FOREACH ( T ${TEST} )

  ADD_EXECUTABLE                ( "${T}" "${T}.cpp" )

  SET_TARGET_PROPERTIES (
    ${T}                        PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/test"
    COMPILE_FLAGS		"${EggCxxFlags}"
    LINK_FLAGS                  "${LINK_FLAGS} ${EggLdFlags}" )

  ADD_DEPENDENCIES      ( "${T}" ${LibraryName}		)
  TARGET_LINK_LIBRARIES ( "${T}" ${LibraryName}		)

  ADD_TEST(
    NAME              "${T}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    COMMAND           "${T}"
  )

ENDFOREACH ()

# End of file
