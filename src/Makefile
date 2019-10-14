CPP=g++
CPPFLAGS=-std=c++11 -Wall -pedantic 
FILENAMES=Makefile isaclient.cpp isaclient.h isaserver.h isaserver.cpp README manual.pdf

all: isaclient isaserver clean

isaclient: isaclient.cpp
	$(CPP) $(CPPFLAGS)  isaclient.cpp isaclient.h -o isaclient

isaserver: isaserver.cpp
	$(CPP) $(CPPFLAGS)  isaserver.cpp isaserver.h -lpthread -pthread -o isaserver

clean:
	rm -f *.o

pack:
	tar -cvf /home/martin/Desktop/xvlach18.tar $(FILENAMES)

rebuild: removebin all

removebin:
	rm -f isaclient isaserver

test:


