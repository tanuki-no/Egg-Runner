# Project presets
#
# Configurure the server now as: "cmake <OPTIONS> <FLAGS>  .."
#
# Options:
#
# Flags:
#
# -DCMAKE_CXX_COMPILER= /usr/bin/g++
# -DCMAKE_INSTALL_PREFIX=/opt/phoenix
# -DCMAKE_BUILD_TYPE=debug | -DCMAKE_BUILD_TYPE=release

# Commons
PROJECT("Egg::Runner" )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

# Describe project
SET ( PROJECT_VERSION_MAJOR     0                                       )
SET ( PROJECT_VERSION_MINOR     1                                       )
SET ( PROJECT_VERSION_PATCH     1                                       )
SET ( PROJECT_VERSION_TWEAK     0                                       )
SET ( PROJECT_VERSION           ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK} )

STRING( TIMESTAMP TODAY "%d/%m/%Y"					)

SET ( ABIMajor          1                                               )
SET ( ABIMinor          0                                               )
SET ( ABIPatch          0                                               )
SET ( ABIVersion        ${ABIMajor}.${ABIMinor}.${ABIPatch}             )

SET ( LibraryName	"egg-runner"
      CACHE STRING	"Runner library ${PROJECT_VERSION}. ABI: ${ABIVersion}" FORCE )

SET ( Alias		"Go cat go"                                     )
SET ( Author		"Vladislav \\\"Tanuki\\\" Mikhailikov"		)
SET ( Copyright		"GNU GPLv3"					)
SET ( Description	"Egg (Linux Daemon SDK) runner library"		)
SET ( SDKName		"EggSDK"					)

# Common build paramteres
OPTION ( BUILD_SHARED_LIBS    "Build shared libraries if ON or static if OFF" ON  )
OPTION ( BUILD_PKGCONFIG      "Generate pkgconfig configuration files"        ON  )
OPTION ( BUILD_TESTS          "Build tests"                                   OFF )

# GNU install dirs
INCLUDE ( GNUInstallDirs )

# Project directories
SET ( Project_Include_Dir "${CMAKE_SOURCE_DIR}/include"		)
SET ( Project_Source_Dir  "${CMAKE_SOURCE_DIR}/source"		)

# Library search paths
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /lib"		)
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /lib32"	)
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /lib64"	)
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /usr/lib"	)
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /usr/lib32"	)
SET ( LIBRARY_SEARCH_PATH "${LIBRARY_SEARCH_PATH} /usr/lib64"	)

# Check for threads
INCLUDE ( FindThreads )
IF ( CMAKE_HAVE_PTHREAD_H )
  SET ( PTHREAD_LIBRARY   ${CMAKE_THREAD_LIBS_INIT} CACHE INTERNAL "POSIX Threads library" )
  SET ( HAVE_PTHREAD      1 )
  SET ( HAVE_LIBPTHREAD   1 )
  MESSAGE ( STATUS "Enabling POSIX Threads" )
ENDIF ()

# Check for librt
CHECK_LIBRARY_EXISTS ("rt" "clock_gettime" ${LIBRARY_SEARCH_PATH} HAVE_LIBRT)

# Configure system
INCLUDE ( module/ConfigureSystem.cmake )

# Load cmake script
INCLUDE ( ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cmake/${SDKName}/egg-common.cmake )
IF ( NOT EggFound )
  MESSAGE ( ERROR "Phoenix::Egg common configuration not found!" )
ELSE ()
  MESSAGE ( STATUS "Phoenix::Egg common configuration found!" )
ENDIF ()

# Load cmake script
INCLUDE ( ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cmake/${SDKName}/egg-variable.cmake )
IF ( NOT EggVariableIncludePath OR
     NOT EggVariableLibraryPath OR
     NOT EggVariableLibraryName )
  MESSAGE ( ERROR "Phoenix::Egg variable library not found!" )
ELSE ()
  FIND_LIBRARY (
    VARIABLE_LIBRARY
    ${EggVariableLibraryName}
    ${EggVariableLibraryPath} )
  MESSAGE ( STATUS "Phoenix::Egg variable library found!" )
ENDIF ()

# Load cmake script
INCLUDE ( ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cmake/${SDKName}/egg-registry.cmake )
IF ( NOT EggRegistryIncludePath OR
     NOT EggRegistryLibraryPath OR
     NOT EggRegistryLibraryName OR
     NOT EggRegistryFullLibraryName )
  MESSAGE ( ERROR "Phoenix::Egg registry library not found!" )
ELSE ()
  FIND_LIBRARY (
    REGISTRY_LIBRARY
    ${EggRegistryFullLibraryName}
    ${EggRegistryLibraryPath} )
  MESSAGE ( STATUS "Phoenix::Egg registry library found!" )
ENDIF ()

# Load cmake script
INCLUDE ( ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cmake/${SDKName}/egg-command-line.cmake )
IF ( NOT EggCommandLineIncludePath OR
     NOT EggCommandLineLibraryPath OR
     NOT EggCommandLineLibraryName )
  MESSAGE ( ERROR "Phoenix::Egg command line library not found!" )
ELSE ()
  FIND_LIBRARY (
    COMMAND_LINE_LIBRARY
    ${EggCommandLineLibraryName}
    ${EggCommandLineLibraryPath} )
  MESSAGE ( STATUS "Phoenix::Egg command line library found!" )
ENDIF ()

# Include pkg-config
INCLUDE ( FindPkgConfig )

IF (PKG_CONFIG_EXECUTABLE)

  # Detect LInux Capability NG library
  PKG_CHECK_MODULES ( "CAP" REQUIRED libcap-ng	)

ELSE ()

  FIND_FILE (
    CAP_INCLUDE_HEADER 
    NAMES	"cap-ng.h"
    PATHS	${CMAKE_INSTALL_FULL_INCLUDEDIR} ${CMAKE_REQUIRED_INCLUDES}
    NO_DEFAULT_PATH )

  GET_FILENAME_COMPONENT (
    CAP_INCLUDE_DIRS ${CAP_INCLUDE_HEADER} DIRECTORY CACHE)

  FIND_LIBRARY (
    CAP_LDFLAGS	"cap-ng"
    PATHS	${CMAKE_INSTALL_FULL_LIBDIR} ${LIBRARY_SEARCH_PATH}
    NO_CMAKE_SYSTEM_PATH )

ENDIF ()

# CMake scripts
CONFIGURE_FILE(
  egg-runner.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/egg-runner.cmake
  @ONLY )

INSTALL(
  FILES       ${CMAKE_CURRENT_BINARY_DIR}/egg-runner.cmake
  DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cmake/${SDKName} )

# PkgConfig
IF (BUILD_PKGCONFIG)

  CONFIGURE_FILE(
    egg-runner.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/egg-runner.pc
    @ONLY )

  INSTALL(
    FILES       ${CMAKE_CURRENT_BINARY_DIR}/egg-runner.pc
    DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)

ENDIF ()

# Includes
ADD_SUBDIRECTORY ( include )

# Sources
ADD_SUBDIRECTORY ( source )

# Tests
IF (BUILD_TESTS)
  ENABLE_TESTING()
  INCLUDE (CTest)
  ADD_SUBDIRECTORY ( test )
ENDIF ()

# End of file
