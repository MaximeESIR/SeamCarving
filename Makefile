CC      = g++
OPTIONS = -c -g -static
SOURCES = .
INC     = .

####### Build rules

all: tache.exe

tache.exe: tpSeam.o imagesRW.o
	$(CC) -g -o tache.exe tpSeam.o imagesRW.o
	del *.o

tpSeam.o: $(SOURCES)/tpSeam.cpp
	$(CC) $(OPTIONS) -I$(INC) $(SOURCES)/tpSeam.cpp

imagesRW.o: $(SOURCES)/imagesRW.cpp
	$(CC) $(OPTIONS) -I$(INC) $(SOURCES)/imagesRW.cpp

run: tache.exe
	tache.exe