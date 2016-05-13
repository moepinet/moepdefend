
SUBDIRS =

AM_CFLAGS = -O2 -Wall

bin_PROGRAMS = defender

defender_SOURCES  = src/defender.c
defender_SOURCES += src/daemonize.h
defender_SOURCES += src/daemonize.c
defender_SOURCES += src/cell.h

defender_CPPFLAGS = $(LIBMOEPCOMMON_CFLAGS)

defender_LDADD  = $(LIBMOEP80211_LIBS)
