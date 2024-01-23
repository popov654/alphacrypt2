CC :=gcc
CPP :=g++
LDFLAGS :=
C_SOURCES :=$(wildcard */*.c)
C_EXECUTABLE :=$(C_SOURCES:.c=)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=)
CPPFLAGS :=-std=c++20
EXECUTABLE :=main.exe
 
all:$(CPP_EXECUTABLE)

$(CPP_EXECUTABLE):$(CPP_SOURCES)
		gcc $(C_SOURCES) -o libs.o
		$(CPP) libs.o $< -g $(LDFLAGS) $(CPPFLAGS) -o $@.exe

clean:
		rm -rf $(EXECUTABLE)
		rm -rf $(wildcard *.exe)
		rm -rf $(wildcard *.o)
		rm -rf $(wildcard */*.exe)
		rm -rf $(wildcard */*.o)