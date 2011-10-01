TARGET = ../../lib/scgi.so

####### Compiler, tools and options

CXX           = g++
DEFINES       =
CFLAGS        = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES) -std=gnu++0x
LINK          = g++
LFLAGS        = -Wl,--hash-style=gnu -Wl,--as-needed -Wl,-O1 -shared
STRIP         = strip

ATL_INC       = -I../../ATL/include
CWGI_INC      = -I../../CWGI/include
CWGI_LIB      = -lCWGI -L../../CWGI/lib
DL_INC        =
DL_LIB        = -ldl

INCPATH = $(CWGI_INC)
####### Build rules

all: $(TARGET)

scgi.o: scgi.cpp ../proto.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o scgi.o scgi.cpp

$(TARGET): scgi.o
	mkdir -p ../../lib
	$(LINK) $(LFLAGS) -o $(TARGET) scgi.o $(CWGI_LIB) -pthread
	$(STRIP) $(TARGET)

clean:
	-rm -f *.o *~

distclean: clean
	-rm -f $(TARGET)
