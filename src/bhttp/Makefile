TARGET = ../../lib/bhttp.so

####### Compiler, tools and options

CXX           = g++
DEFINES       =
CFLAGS        = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES) -std=gnu++0x
LINK          = g++
LFLAGS        = -Wl,--hash-style=gnu -Wl,--as-needed -Wl,-O1 -shared
STRIP         = strip

CWGI_INC      = -I../../CWGI/include
CWGI_LIB      = -lCWGI -L../../CWGI/lib

INCPATH = $(CWGI_INC)
####### Build rules

all: $(TARGET)

bhttp.o: bhttp.cpp ../proto.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o bhttp.o bhttp.cpp

$(TARGET): bhttp.o
	mkdir -p ../../lib
	$(LINK) $(LFLAGS) -o $(TARGET) bhttp.o $(CWGI_LIB) -pthread
	$(STRIP) $(TARGET)

clean:
	-rm -f *.o *~

distclean: clean
	-rm -f $(TARGET)
