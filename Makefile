CC :=gcc
CPP :=g++
LDFLAGS :=
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:main.c=)
CFLAGS :=-std=c99
EXECUTABLE :=acp.exe
 
all:
		$(CC) $(CFLAGS) md5.c acp.c main.c -o $(EXECUTABLE)
		$(CC) $(CFLAGS) md5.c acp.c tests.c -o tests

clean:
		rm -rf $(EXECUTABLE)
		rm -rf $(wildcard *.exe)
		rm -rf $(wildcard *.o)