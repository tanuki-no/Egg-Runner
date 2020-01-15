# Copyright (c) 2020, Vladislav G. Mikhailikov, <vmikhailikov@gmail.com>

INCLUDE ( CheckIncludeFile )
INCLUDE ( CheckFunctionExists )

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  CHECK_INCLUDE_FILE ( signal.h			HAVE_SIGNAL_H )
  IF(NOT HAVE_SIGNAL_H)
    MESSAGE(FATAL_ERROR "File signal.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( sys/types.h		HAVE_SYS_TYPES_H )
  IF(NOT HAVE_SYS_TYPES_H)
    MESSAGE(FATAL_ERROR "File sys/types.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( sys/stat.h		HAVE_SYS_STAT_H )
  IF(NOT HAVE_SYS_STAT_H)
    MESSAGE(FATAL_ERROR "File sys/types.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( sys/wait.h		HAVE_SYS_WAIT_H )
  IF(NOT HAVE_SYS_WAIT_H)
    MESSAGE(FATAL_ERROR "File sys/wait.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( fcntl.h			HAVE_FCNTL_H )
  IF(NOT HAVE_FCNTL_H)
    MESSAGE(FATAL_ERROR "File fcntl.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( pwd.h			HAVE_PWD_H )
  IF(NOT HAVE_PWD_H)
    MESSAGE(FATAL_ERROR "File pwd.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( grp.h			HAVE_GRP_H )
  IF(NOT HAVE_GRP_H)
    MESSAGE(FATAL_ERROR "File grp.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_FUNCTION_EXISTS ( getuid		HAVE_GETUID )
  IF (NOT HAVE_GETUID)
    MESSAGE (FATAL_ERROR "getuid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( geteuid		HAVE_GETEUID )
  IF (NOT HAVE_GETEUID)
    MESSAGE (FATAL_ERROR "geteuid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( setuid		HAVE_SETUID )
  IF (NOT HAVE_SETUID)
    MESSAGE (FATAL_ERROR "setuid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( seteuid		HAVE_SETEUID )
  IF (NOT HAVE_SETEUID)
    MESSAGE (FATAL_ERROR "seteuid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( getgid		HAVE_GETGID )
  IF (NOT HAVE_GETGID)
    MESSAGE (FATAL_ERROR "getgid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( getegid		HAVE_GETEGID )
  IF (NOT HAVE_GETEGID)
    MESSAGE (FATAL_ERROR "getegid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( setgid		HAVE_SETGID )
  IF (NOT HAVE_SETGID)
    MESSAGE (FATAL_ERROR "setgid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( setegid		HAVE_SETEGID )
  IF (NOT HAVE_SETEUID)
    MESSAGE (FATAL_ERROR "setegid() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( initgroups		HAVE_INITGROUPS )

  CHECK_FUNCTION_EXISTS (getpwnam		HAVE_PWNAM )
  CHECK_FUNCTION_EXISTS (getpwnam_r		HAVE_PWNAM_R )
  IF (NOT HAVE_PWNAM AND NOT HAVE_PWNAM_R)
    MESSAGE (FATAL_ERROR "getpwnam() or getpwnam_r() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS (getpwuid		HAVE_PWUID )
  CHECK_FUNCTION_EXISTS (getpwuid_r		HAVE_PWUID_R )
  IF (NOT HAVE_PWUID AND NOT HAVE_PWUID_R)
    MESSAGE (FATAL_ERROR "getpwuid() or getpwuid_r() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS (getgrnam		HAVE_GRNAM )
  CHECK_FUNCTION_EXISTS (getgrnam_r		HAVE_GRNAM_R )
  IF (NOT HAVE_GRNAM AND NOT HAVE_GRNAM_R)
    MESSAGE (FATAL_ERROR "getgrnam() or getgrnam_r() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS (getgrgid		HAVE_GRGID )
  CHECK_FUNCTION_EXISTS (getgrgid_r		HAVE_GRGID_R )
  IF (NOT HAVE_GRGID AND NOT HAVE_GRGID_R)
    MESSAGE (FATAL_ERROR "getgrgid() or getgrgid_r() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_INCLUDE_FILE ( stdlib.h			HAVE_STDLIB_H )
  IF(NOT HAVE_STDLIB_H)
    MESSAGE(FATAL_ERROR "File stdlib.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_FUNCTION_EXISTS ( getenv		HAVE_GETENV )
  IF (NOT HAVE_GETENV)
    MESSAGE (FATAL_ERROR "getenv() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( setenv		HAVE_SETENV )
  IF (NOT HAVE_SETENV)
    MESSAGE (FATAL_ERROR "setenv() not found. Check your system and restart cmake")
  ENDIF ()

  # Check functions
  CHECK_FUNCTION_EXISTS ( __secure_getenv	HAVE_SECURE_CETENV   )
  CHECK_FUNCTION_EXISTS ( _secure_getenv	HAVE_SECURE_CETENV_  )
  CHECK_FUNCTION_EXISTS ( secure_getenv		HAVE_SECURE_CETENV__ )

  CHECK_INCLUDE_FILE ( syslog.h			HAVE_SYSLOG_H )
  IF(NOT HAVE_SYSLOG_H)
    MESSAGE(FATAL_ERROR "File syslog.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_INCLUDE_FILE ( unistd.h			HAVE_UNISTD_H )
  IF(NOT HAVE_UNISTD_H)
    MESSAGE(FATAL_ERROR "File unistd.h not found but required. Check your system configuration")
  ENDIF()

  CHECK_FUNCTION_EXISTS ( timegm		HAVE_TIMEGM )
  IF (NOT HAVE_TIMEGM)
    MESSAGE (FATAL_ERROR "timegm() not found. Check your system and restart cmake")
  ENDIF ()

  CHECK_FUNCTION_EXISTS ( asctime_r		HAVE_ASCTIME_R      )
  CHECK_FUNCTION_EXISTS ( ctime_r		HAVE_CTIME_R        )
  CHECK_FUNCTION_EXISTS ( gmtime_r		HAVE_GMTIME_R       )
  CHECK_FUNCTION_EXISTS ( localtime_r		HAVE_LOCALTIME_R    )

  # Fork functions
  CHECK_FUNCTION_EXISTS (fork		HAVE_FORK)
  IF(NOT HAVE_FORK)
    MESSAGE(FATAL_ERROR "fork() not found but required. Check your system configuration.")
  ENDIF()

  CHECK_FUNCTION_EXISTS ( vfork		HAVE_VFORK	)
  CHECK_FUNCTION_EXISTS ( clone		HAVE_CLONE	)
  CHECK_FUNCTION_EXISTS ( unshare	HAVE_UNSHARE	)

  SET(__LINUX__ 1 CACHE INTERNAL "Platform macros")

ELSE()

  MESSAGE(FATAL_ERROR "Non-linux configurations not supported right now. Request support if you need that")

ENDIF()

# End of file
