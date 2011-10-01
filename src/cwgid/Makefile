TARGET = ../../bin/cwgid

####### Compiler, tools and options

CXX           = g++
DEFINES       =
CFLAGS        = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES) -std=gnu++0x
LINK          = g++
LFLAGS        = -Wl,--hash-style=gnu -Wl,--as-needed -Wl,-O1
STRIP         = strip

ATL_INC       = -I../../ATL/include
CWGI_INC      = -I../../CWGI/include
CWGI_LIB      = -lCWGI -L../../CWGI/lib
DL_INC        =
DL_LIB        = -ldl

INCPATH = $(CWGI_INC) $(DL_INC)
####### Build rules

all: ../../bin/cwgid

cwgid.o: cwgid.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o cwgid.o cwgid.cpp

$(TARGET): cwgid.o
	mkdir -p ../../bin
	$(LINK) $(LFLAGS) -o $(TARGET) cwgid.o $(DL_LIB) -pthread
	$(STRIP) $(TARGET)

clean:
	-rm -f *.o *~

distclean: clean
	-rm -f $(TARGET)
