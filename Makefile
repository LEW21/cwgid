DESTDIR = /usr/local

all: bin/cwgid lib/scgi.so lib/bhttp.so

CWGI:
	git clone git://github.com/LEW21/cwgi.git CWGI
	cd CWGI && make

bin/cwgid: CWGI
	cd src/cwgid && make

lib/scgi.so: CWGI
	cd src/scgi && make

lib/bhttp.so: CWGI
	cd src/bhttp && make

clean:
	-rm -f *~
	-find . -name *~ -exec rm -f {} \;
	-rm -Rf CWGI
	cd src && make clean

distclean: clean
	cd src && make distclean
	rm -rf bin/ lib/

install: bin/cwgid lib/scgi.so lib/bhttp.so
	mkdir -p $(DESTDIR)/bin
	cp bin/cwgid* $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/lib/cwgid
	cp lib/*.so* $(DESTDIR)/lib/cwgid
	mkdir -p $(DESTDIR)/include/CWGI
	cp include/CWGI/cwgid.h $(DESTDIR)/include/CWGI

uninstall:
	rm $(DESTDIR)/include/CWGI/cwgid.h
	rm $(DESTDIR)/bin/cwgid*
	rm -R $(DESTDIR)/lib/cwgid
