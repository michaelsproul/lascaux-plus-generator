CXX=clang++
CXXFLAGS=-W -Wall -std=c++11 -I /usr/include/freetype2
LDFLAGS=-lfreetype -lm

default: generate
	./generate

generate: generate.cpp

clean:
	rm -f snapshot.json generate

.PHONY: default clean
