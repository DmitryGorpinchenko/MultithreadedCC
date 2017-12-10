CC=g++ -std=c++14
OBJECTS=main.o chat.o
EXE=chat

all: $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXE)

main.o: chat.h main.cpp
	$(CC) -c main.cpp

chat.o: chat.h chat.cpp
	$(CC) -c chat.cpp
	
clean:
	rm -rf core *.o $(EXE)
