# change application name here (executable output name)
TARGET=glade-sample

# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall

PTHREAD=-pthread

OPENCVLIB=

GTKLIB=`pkg-config --cflags --libs gtk+-3.0` 
GSTLIB=`pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0`

# Check for OpenCV being installed. 
PACK=opencv
PACK:=$(shell pkg-config --exists $(PACK) && echo '$(PACK)')
ifneq ($(PACK),)
	OPENCVLIB=`pkg-config --cflags --libs $(PACK);` 
endif

ifneq ($(OPENCVLIB), )
	FLAGS = -DHAVEOPENCV
else
$(info OpenCV not found )
endif

TCAMLIB=`pkg-config --cflags --libs tcam` -rdynamic 

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) $(FLAGS) -pipe -rdynamic 

# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) $(GSTLIB) $(TCAMLIB) $(OPENCVLIB) -export-dynamic -rdynamic -lstdc++ 

OBJS=    main.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: src/main.cpp
	$(CC) -c $(CCFLAGS)  src/main.cpp $(GTKLIB) $(GSTLIB) $(TCAMLIB) $(OPENCVLIB) -o main.o

clean:
	rm -f *.o $(TARGET)
