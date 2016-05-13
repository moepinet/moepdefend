AC_INIT([moep80211 DEFENDER], [0.0], [moepi@moepi.net], [defender], [http://moep80211.net])
#AC_PREREQ([2.69])

AC_PROG_CC
AC_PROG_LN_S

AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects foreign])
AM_SILENT_RULES([yes])

AM_PROG_AR
AM_PROG_CC_C_O	# Needed for some buggy automake versions

LT_INIT([disable-shared])

PKG_CHECK_MODULES([LIBMOEP80211], [libmoep80211 >= 1.0])

AC_SEARCH_LIBS([exp], [m])
AC_SEARCH_LIBS([timer_create], [rt])
AC_SEARCH_LIBS([timer_delete], [rt])
AC_SEARCH_LIBS([timer_settime], [rt])
AC_SEARCH_LIBS([timer_gettime], [rt])

LIBMOEPCOMMON_CFLAGS="-I\$(top_srcdir)/libmoepcommon/include"
AC_SUBST(LIBMOEPCOMMON_CFLAGS)

AC_CONFIG_FILES([Makefile])

AC_CONFIG_SUBDIRS([])

AC_OUTPUT
