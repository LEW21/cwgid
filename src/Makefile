all: ../bin/cwgid ../lib/scgi.so

../bin/cwgid:
	cd cwgid && make

../lib/scgi.so:
	cd scgi && make

../lib/bhttp.so:
	cd bhttp && make

clean:
	-rm -f *.o *~
	cd cwgid && make clean
	cd scgi && make clean
	cd bhttp && make clean

distclean: clean
	-rm -f ../bin ../lib
	cd cwgid && make distclean
	cd scgi && make distclean
	cd bhttp && make distclean
