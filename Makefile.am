
SUBDIRS =

AM_CFLAGS = -O2 -Wall

bin_PROGRAMS = moepdefend

moepdefend_SOURCES  = src/defender.c
moepdefend_SOURCES += src/daemonize.h
moepdefend_SOURCES += src/daemonize.c
moepdefend_SOURCES += src/cell.h
moepdefend_SOURCES += src/cell.c
moepdefend_SOURCES += src/sta.h
moepdefend_SOURCES += src/sta.c

moepdefend_CPPFLAGS = $(LIBMOEPCOMMON_CFLAGS)

moepdefend_LDADD  = $(LIBMOEP80211_LIBS)
