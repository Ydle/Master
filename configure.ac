dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Master, 0.5)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_LANG([C++])
AC_PROG_CXX

AM_CXXFLAGS="-Wall -std=gnu++0x -fpermissive"


PKG_CHECK_MODULES([JSONCPP], [jsoncpp], [],[
	AC_MSG_ERROR([jsoncpp is not installed.])
])
PKG_CHECK_MODULES([LIBCONFIG], [libconfig++], [],[
	AC_MSG_ERROR([libconfig++ is not installed.])
])


#Check for libs
#-lwiringPi  -lpthread -lmicrohttpd -ljsoncpp -lcurl -lconfig++
AC_CHECK_LIB(pthread, pthread_create, [], [
	AC_MSG_ERROR([pthread is not installed.])
])
AC_SEARCH_LIBS(curl_easy_init, curl, [], [
	AC_MSG_ERROR([libcurl is not installed.])
])
AC_SEARCH_LIBS(wiringPiSetup, wiringPi, [], [
	AC_MSG_ERROR([libwiringPi is not installed.])
])

AC_SEARCH_LIBS(MHD_start_daemon, microhttpd, [], [
	AC_MSG_ERROR([libmicrohttpd is not installed.])
])

AC_SUBST(AM_CXXFLAGS)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

