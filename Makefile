all:    libs

install: libs
	cp libVT.so /usr/local/lib/

libs:   libVT.so

libVT.so: libVT.c
	rm -f libVT.so*
	gcc -O3 -fPIC -shared -Werror -Wall -o libVT.so  libVT.c -ldl

clean:
	rm -f libVT.so*
