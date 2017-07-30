CC=gcc
OBJECTS=main.o chat.o
EXE=main

all: $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE)

main.o: chat.h main.c
	$(CC) -c main.c

chat.o: chat.h chat.c
	$(CC) -c chat.c
	
clean:
	rm -rf core *.o $(EXE)
