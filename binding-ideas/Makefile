

efl.so: efl.c
	gcc -g -Os -o efl.so -pthread -shared -fpic efl.c $(shell pkg-config lua5.1 evas --cflags --libs)

clean:
	rm -f efl.so
