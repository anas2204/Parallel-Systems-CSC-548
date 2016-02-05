EXECUTABLE      := p2_mpi.c p2_func.c
OUTPUT := p2

CXX        := g++
CC         := gcc
MPICC         := mpicc
LINK       := g++ -fPIC

INCLUDES  += -I. -I/ncsu/gcc346/include/c++/ -I/ncsu/gcc346/include/c++/3.4.6/backward 
LIB       := -L/ncsu/gcc346/lib

default:
	$(MPICC) -g $(EXECUTABLE) -o $(OUTPUT) -lm

clean:
	rm -f $(OUTPUT)

