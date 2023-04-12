## This is a simple Makefile 

# Define what compiler to use and the flags.
CC=cc
CXX=g++
CCFLAGS= -g -std=c++11 -Wall -Werror
LDLIBS= -lm
#all: test_list

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.cpp
	$(CXX) -c $(CCFLAGS) $<

###################################
# BEGIN SOLUTION
proj: proj.o Pipeline.o Instruction.o DepMap.o
	$(CXX) -o proj proj.o Pipeline.o Instruction.o DepMap.o $(CCFLAGS) $(LDLIBS)

Pipline.o: Instruction.h DepMap.h
DepMap.o: Instruction.h
clean:
	rm -f core *.o proj

