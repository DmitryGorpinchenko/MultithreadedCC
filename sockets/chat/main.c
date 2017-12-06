#include <stdio.h>

#include "chat.h"

int main(int argc, char **argv) 
{	
	struct Chat *  chat;
	unsigned short port;
	int            max_clients;
	
	if (argc < 3
		|| sscanf(argv[1], "%d", &max_clients) != 1
		|| sscanf(argv[2], "%hu", &port) != 1
		|| (chat = create_chat(max_clients, port)) == NULL)
		return -1;
	
	run_chat(chat);
	free_chat(chat);
	return 0;
}

