OUTPUT := p1
INPUT = p1.c

CXX        := g++
CC         := gcc
MPICC         := mpicc
LINK       := g++ -fPIC

INCLUDES  += -I. -I/ncsu/gcc346/include/c++/ -I/ncsu/gcc346/include/c++/3.4.6/backward 
LIB       := -L/ncsu/gcc346/lib

default:
	$(MPICC) -g -o $(OUTPUT) $(INPUT)

clean:
	rm -f $(OUTPUT)
