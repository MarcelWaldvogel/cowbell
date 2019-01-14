LIBS	= -leXosip2 -losip2 -losipparser2 -lcares -lsndfile -lortp ${INILIB} -lm
CFLAGS	= -g -Iflexosip -I/usr/local/include ${INIINC} -Wall -Wextra -O -DENABLE_TRACE
LDFLAGS	= -g -L/usr/local/lib -Xlinker -rpath=/usr/local/lib
ifeq (arm, $(word 1,$(subst -, ,${MAKE_HOST})))
# ${MAKE_HOST} is e.g. arm-unknown-linux-gnueabihf, assume we are on a Raspberry Pi
LDFLAGS += -lwiringPi
endif

.PHONY: all clean
all:	cowbell

cowbell:cowbell.o action.o flexosip/flexosip.a
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

cowbell.o action.o: action.h

# Try to build it every time
flexosip/flexosip.a: $(wildcard flexosip/*.[ch])
	${MAKE} -C flexosip

clean:
	${RM} *.o *.a
	${MAKE} -C flexosip $@

# Decide between system-installed or local inih
# This is at the bottom to not make any of this the default target
ifeq ($(wildcard flexosip/inih/ini.[c]), flexosip/inih/ini.c)
INILIB  = # Will be included through the dependency; inih/ini.o
INIINC  = -Iflexosip/inih

cowbell:	flexosip/inih/ini.o
cowbell.o:	flexosip/inih/ini.h
inih/ini.o:	flexosip/inih/ini.c flexosip/inih/ini.h
else
INILIB  = -linih
endif
