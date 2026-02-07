.PHONY: debug

dist/release_zpen dist/debug_zpen: src/zpen.c src/stb_image_write.h
	mkdir -p dist
	echo "*" > dist/.gitignore
	$(CC) $(CFLAGS) -o dist/release_zpen src/zpen.c -lX11 -lXrender -lm
	$(CC) $(CFLAGS) -g -o dist/debug_zpen src/zpen.c -lX11 -lXrender -lm

debug: dist/debug_zpen
	gdb ./dist/debug_zpen
