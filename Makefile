DESTDIR = /usr/local

all: bin/cwgid lib/scgi.so

ATL:
	git clone git://gitorious.org/atl/atl.git ATL

CWGI:
	git clone git://gitorious.org/cwgi/cwgi.git CWGI
	cd CWGI && make

bin/cwgid: CWGI
	cd src/cwgid && make

bin/cwgid-launch: CWGI
	cd src/cwgid-launch && make

lib/scgi.so: CWGI
	cd src/scgi && make

clean:
	-rm -f *~
	-find . -name *~ -exec rm -f {} \;
	-rm -Rf ATL
	-rm -Rf CWGI
	cd src && make clean

distclean: clean
	cd src && make distclean
	rm -rf bin/ lib/

install: bin/cwgid lib/scgi.so
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
