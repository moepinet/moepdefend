
SUBDIRS =

AM_CFLAGS = -O2 -Wall

bin_PROGRAMS = moepdefend

moepdefend_SOURCES  = src/defender.c
moepdefend_SOURCES += src/defender.h
moepdefend_SOURCES += src/args.c
moepdefend_SOURCES += src/args.h
moepdefend_SOURCES += src/daemonize.h
moepdefend_SOURCES += src/daemonize.c
moepdefend_SOURCES += src/cell.h
moepdefend_SOURCES += src/cell.c
moepdefend_SOURCES += src/sta.h
moepdefend_SOURCES += src/sta.c
moepdefend_SOURCES += src/whitelist.h
moepdefend_SOURCES += src/whitelist.c
moepdefend_SOURCES += src/radiotap.c
moepdefend_SOURCES += src/radiotap.h
moepdefend_SOURCES += src/helper.c
moepdefend_SOURCES += src/helper.h
moepdefend_SOURCES += src/list_sort.c
moepdefend_SOURCES += src/list_sort.h
moepdefend_SOURCES += src/state.c
moepdefend_SOURCES += src/state.h
moepdefend_SOURCES += src/attack.c
moepdefend_SOURCES += src/attack.h
moepdefend_SOURCES += src/frametypes.h
moepdefend_SOURCES += src/params.h
moepdefend_SOURCES += src/attacks/deauth.c
moepdefend_SOURCES += src/attacks/deauth.h
moepdefend_SOURCES += src/attacks/michael.c
moepdefend_SOURCES += src/attacks/michael.h
moepdefend_SOURCES += src/attacks/myattack.c
moepdefend_SOURCES += src/attacks/myattack.h

moepdefend_CPPFLAGS = $(LIBMOEPCOMMON_CFLAGS)

moepdefend_LDADD  = $(LIBMOEP80211_LIBS)
