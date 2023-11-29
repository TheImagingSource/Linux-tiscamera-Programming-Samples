# change application name here (executable output name)
TARGET=arv-polarcamera

# compiler
CC=g++
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall

PTHREAD=-pthread


OPENCVLIB=`pkg-config --libs opencv4` 
OPENCVINC=`pkg-config --cflags opencv4` 
GLIB=`pkg-config --libs glib-2.0` 
GINC=`pkg-config --cflags  glib-2.0` 
ARVLIB=`pkg-config --libs aravis-0.6`
ARVINC=`pkg-config --cflags aravis-0.6`
                                             
CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) $(FLAGS) -pipe  

LIBS=$(PTHREAD) $(OPENCVLIB) $(ARVLIB) $(GLIB) -lstdc++ 
INCLUDES=$(OPENCVINC) $(ARVINC) $(GINC) 

OBJS= main.o arvhelper.o
SOURCES=  main.cpp arvhelper.cpp

all: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(INCLUDES)

main.o: main.cpp arvhelper.cpp arvhelper.h
	$(CC) -g $(CCFLAGS) $(INCLUDES) -c  main.cpp

arvhelper.o: arvhelper.cpp arvhelper.h
	$(CC) -g $(CCFLAGS) $(INCLUDES) -c  arvhelper.cpp


clean:
	rm -f *.o $(TARGET)
