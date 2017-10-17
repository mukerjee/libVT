all: libVT.so

install: libVT.so
	cp libVT.so /usr/lib/

libVT.so: libVT.c
	rm -f libVT.so*
	gcc -O3 -fPIC -shared -Werror -Wall -o libVT.so  libVT.c -ldl

clean:
	rm -f libVT.so*
