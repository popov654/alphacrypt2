CC :=gcc
CPP :=g++
LDFLAGS :=
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:main.c=)
CFLAGS :=-std=c99
EXECUTABLE :=main.exe
 
all:$(C_EXECUTABLE)

$(C_EXECUTABLE):$(C_SOURCES)
		$(CC) $(CFLAGS) $(C_SOURCES) -o $(EXECUTABLE)

clean:
		rm -rf $(EXECUTABLE)
		rm -rf $(wildcard *.exe)
		rm -rf $(wildcard *.o)