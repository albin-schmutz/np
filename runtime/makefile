CC = gcc
CFLAGS = -m32 -Wall

ifdef SystemRoot
	RM = del /Q *.o npx.exe
else
	RM = rm -f *.o npx
endif

NPX = npx.o

all: npx

npx: $(NPX)
	$(CC) $(CFLAGS) -o npx $(NPX)

npx.o: npx.c opcodes.h
	$(CC) $(CFLAGS) -c npx.c

clean:
	$(RM)
