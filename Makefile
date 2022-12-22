CC        ?= cc
CFLAGS     = -std=c99 -O2 $(shell pkg-config libyuv --cflags) $(shell pkg-config liby4m --cflags) -fPIC -Wall -Wextra -Wpedantic -g
LDFLAGS    = $(shell imlib2-config --libs) $(shell pkg-config libyuv --libs) $(shell pkg-config liby4m --libs)
PLUGINDIR ?= $(shell pkg-config --variable=libdir imlib2)/imlib2/loaders/

all:
	${CC} -shared -o y4m.so ${CFLAGS} ${LDFLAGS} *.c

clean:
	rm -f y4m.so

install:
	mkdir -p ${DESTDIR}${PLUGINDIR}
	install -m 755 y4m.so ${DESTDIR}${PLUGINDIR}

uninstall:
	rm ${PLUGINDIR}/y4m.so
