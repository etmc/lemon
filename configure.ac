AC_PREREQ(2.53)
AC_INIT([Lemon], [0.99], [a.deuzeman@rug.nl], [lemon])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/header_construct.c])
AC_CONFIG_HEADERS([include/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([mpi], [MPI_File_set_view])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADERS([stdint.h],
[ dnl for inttypes.h and stdint.h for uint_xxx types
  dnl if successful check for the actual types too
  AC_CHECK_TYPES([uint16_t, uint32_t, uint64_t],
                 [],
                 [AC_MSG_ERROR([stdint.h found but either uint16_t, uint32_t or uint64_t not found]) ]
                )
],
[AC_MSG_ERROR([stdint.h is a required header])]
)

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT
