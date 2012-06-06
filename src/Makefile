all: ../bin/cwgid ../bin/cwgid-launch ../lib/scgi.so

../bin/cwgid:
	cd cwgid && make

../bin/cwgid-launch:
	cd cwgid-launch && make

../lib/scgi.so:
	cd scgi && make

clean:
	-rm -f *.o *~
	cd cwgid && make clean
	cd cwgid-launch && make clean
	cd scgi && make clean

distclean: clean
	-rm -f ../bin ../lib
	cd cwgid && make distclean
	cd cwgid-launch && make distclean
	cd scgi && make distclean
