# Public headers

# Private headers
CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/common.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/common.h"
  NEWLINE_STYLE UNIX )

# Copy Egg::Runner library includes
FILE (
	COPY "${CMAKE_CURRENT_SOURCE_DIR}/egg/runner/credentials.hpp"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/egg/runner" )

FILE (
	COPY "${CMAKE_CURRENT_SOURCE_DIR}/egg/runner/environment.hpp"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/egg/runner" )

FILE (
	COPY "${CMAKE_CURRENT_SOURCE_DIR}/egg/runner/filesystem.hpp"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/egg/runner" )

FILE (
	COPY "${CMAKE_CURRENT_SOURCE_DIR}/egg/runner/runner.hpp"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/egg/runner" )

FILE (
	COPY "${CMAKE_CURRENT_SOURCE_DIR}/egg/runner/signal.hpp"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/egg/runner" )

# Egg public includes
SET (
  Public_Include

  "${CMAKE_CURRENT_BINARY_DIR}/egg/runner/environment.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/egg/runner/filesystem.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/egg/runner/runner.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/egg/runner/signal.hpp"

  CACHE INTERNAL "Common headers" )

# Install the Egg::Runner library
INSTALL(
  FILES       ${Public_Include}
  DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/egg/runner" )

# End of file
