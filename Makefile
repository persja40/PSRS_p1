all:
	make clean; make program; make run

# dla C++ wszedzie zmieniamy .c na .cpp
MAKEFLAGS = --no-print-directory

CC= mpic++		# dla C++:   CC=g++
CFLAGS= -O2 -std=c++14
INCLUDE=
LIB= #-lpthread -lm -lgsl -lgslcblas # dla lapacka:	LIB= -lm -llapack -lblas
SOURCES= 
OBJECTS= $(SOURCES:.cpp=.o)

TARGET = program

$(TARGET): main.cpp $(OBJECTS)
	$(CC) -O2 -std=c++14 -o $@ $^ $(LIB)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

#run on one maschine
run:
	./program

.PHONY: clean

clean:
	rm -f  $(TARGET) *.o
