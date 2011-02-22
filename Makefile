
# dependencies: imlib2
# debian: sudo dpkg -i libimlib2-dev
# freebsd: cd /usr/ports/graphics/imlib2; sudo make install
CFLAGS+=-Wall
LDFLAGS+=-lX11 -lImlib2

all: cslides
clean:
	rm -f cslides

run: cslides
	find $(HOME)/Pictures -name '*.jpg' | ./cslides

install:
	cp -f cslides /usr/local/bin/
