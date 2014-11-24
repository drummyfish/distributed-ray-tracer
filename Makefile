# Procedural texture generator project
# Miloslav Ciz, 2012

CC=gcc
CC2=c++
CFLAGS=-std=c99 -g -pedantic -Wall -Wextra
CFLAGS2=-std=c++11 -Wall -pedantic
SOURCEDIR=src

all: main.o colorbuffer.o lodepng.o raytracer.o
	$(CC2) $(CFLAGS2) -lm -o program.exe main.o colorbuffer.o lodepng.o obj.o obj_parser.o raytracer.o

main.o: $(SOURCEDIR)/main.cpp
	$(CC2) $(CFLAGS2) -c -o main.o $(SOURCEDIR)/main.cpp

lodepng.o: $(SOURCEDIR)/lodepng.c $(SOURCEDIR)/lodepng.h
	$(CC) $(CFLAGS) -c -o lodepng.o $(SOURCEDIR)/lodepng.c

colorbuffer.o: $(SOURCEDIR)/colorbuffer.c $(SOURCEDIR)/colorbuffer.h $(SOURCEDIR)/lodepng.h
	$(CC) $(CFLAGS) -c -o colorbuffer.o $(SOURCEDIR)/colorbuffer.c

raytracer.o: $(SOURCEDIR)/raytracer.cpp $(SOURCEDIR)/raytracer.hpp
	$(CC2) $(CFLAGS2) -c -o raytracer.o $(SOURCEDIR)/raytracer.cpp   