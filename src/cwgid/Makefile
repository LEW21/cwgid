TARGET = ../../bin/cwgid

####### Compiler, tools and options

CXX           = g++
DEFINES       =
CFLAGS        = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -fno-strict-aliasing -Wall -W -D_REENTRANT -fPIC $(DEFINES) -std=gnu++0x
LINK          = g++
LFLAGS        = -Wl,--hash-style=gnu -Wl,--as-needed -Wl,-O1
STRIP         = strip

CWGI_INC      = -I../../CWGI/include
CWGI_LIB      = -lCWGI -L../../CWGI/lib
DL_INC        =
DL_LIB        = -ldl
SD_INC        =
SD_LIB        = -lsystemd-daemon

INCPATH = $(CWGI_INC) $(DL_INC) $(SD_INC)
####### Build rules

all: $(TARGET)

cwgid.o: cwgid.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o cwgid.o cwgid.cpp

$(TARGET): cwgid.o
	mkdir -p ../../bin
	$(LINK) $(LFLAGS) -o $(TARGET) cwgid.o $(DL_LIB) $(SD_LIB) -pthread
	$(STRIP) $(TARGET)

clean:
	-rm -f *.o *~

distclean: clean
	-rm -f $(TARGET)
