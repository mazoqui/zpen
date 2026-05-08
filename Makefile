PREFIX ?= /usr/local
DESTDIR ?=

all: dist/release_zpen dist/debug_zpen

release: dist/release_zpen

dist/release_zpen: src/zpen.c src/stb_image.h src/stb_image_write.h
	mkdir -p dist
	echo "*" > dist/.gitignore
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ src/zpen.c $(LDFLAGS) -lX11 -lXrender -lm

dist/debug_zpen: src/zpen.c src/stb_image.h src/stb_image_write.h
	mkdir -p dist
	echo "*" > dist/.gitignore
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -o $@ src/zpen.c $(LDFLAGS) -lX11 -lXrender -lm

debug: dist/debug_zpen
	gdb ./dist/debug_zpen

install: dist/release_zpen
	install -D -m 0755 dist/release_zpen $(DESTDIR)$(PREFIX)/bin/zpen

clean:
	rm -rf dist

.PHONY: all release debug install clean
