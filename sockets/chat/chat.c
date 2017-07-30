#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "chat.h"

#define MAX_MSG_SIZE    (1 << 10)
#define MAX_EVENTS      32
#define MAX_HEADER_SIZE 10

struct Chat {
	int  epoll;
	int  master;
	int *slaves;
	int  size;
	int  max_size;
};

int set_nonblock(int fd);
struct Chat *create_invalid_chat(void);
int add_client(struct Chat *chat, int client_id);
int remove_client(struct Chat *chat, int client_id);
void broadcast_msg(struct Chat *chat, int fd_from, char *buff, int bytes);

void run_chat(struct Chat *chat)
{	
	static const char         join_err_msg[] = "Sorry, you cannot join this chat now! Please, try again later!";
	static const char         join_str[]     = "New joiner:";
	static const char         leave_str[]    = "Leaved:";
	static const char         recv_err_msg[] = "Unable to recieve data from:";
	static char               buff[MAX_MSG_SIZE];
	static char               msg[MAX_MSG_SIZE];
	static struct epoll_event ev;
	int                       ret_events;
	int                       slave;
	int                       bytes;
	
	ev.events = EPOLLIN;
	
	if (chat == NULL
		|| listen(ev.data.fd = chat->master, SOMAXCONN) < 0
		|| epoll_ctl(chat->epoll, EPOLL_CTL_ADD, chat->master, &ev) < 0)
		return;

	struct epoll_event events[MAX_EVENTS];

	do {
		if ((ret_events = epoll_wait(chat->epoll, events, MAX_EVENTS, -1)) < 0)
			return;		
		for (int i = 0; i < ret_events; i++) {
			if (events[i].data.fd == chat->master) {
				do {
					if ((slave = accept(chat->master, NULL, NULL)) < 0) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) 
							break; // accepted all pending connections at this moment
						continue;
					}
					set_nonblock(slave);
					if (add_client(chat, slave) < 0) {
						snprintf(buff, MAX_MSG_SIZE, "%8d: %s\n", chat->master, join_err_msg);
						send(slave, buff, strlen(buff), MSG_NOSIGNAL);
						close(slave);
					} else {
						snprintf(buff, MAX_MSG_SIZE, "%8d: %s %d\n", chat->master, join_str, slave);
						broadcast_msg(chat, slave, buff, strlen(buff));
					}
				} while (1);
			} else if ((bytes = recv(events[i].data.fd, msg, MAX_MSG_SIZE, 0)) < 0) {
				switch(errno) {
				case EAGAIN:
#if EAGAIN != EWOULDBLOCK
				case EWOULDBLOCK:
#endif
					continue;
				default:
					snprintf(buff, MAX_MSG_SIZE, "%8d: %s %d\n", chat->master, recv_err_msg, events[i].data.fd);
					broadcast_msg(chat, -1, buff, strlen(buff));
					break;
				}
			} else if (bytes > 0) {
				int len = MAX_HEADER_SIZE + bytes + 1 < MAX_MSG_SIZE ? MAX_HEADER_SIZE + bytes + 1 
										     : MAX_MSG_SIZE;
				snprintf(buff, len, "%8d: %s\n", events[i].data.fd, msg);
				broadcast_msg(chat, events[i].data.fd, buff, strlen(buff));
			} else if (remove_client(chat, events[i].data.fd) == 0) {			
				snprintf(buff, MAX_MSG_SIZE, "%8d: %s %d\n", chat->master, leave_str, events[i].data.fd);
				broadcast_msg(chat, -1, buff, strlen(buff));
			}
		}
	} while (1);
}

struct Chat *create_chat(int max_size, short port)
{
	if (max_size <= 0)
		return NULL;

	struct Chat *chat;
	
	if ((chat = create_invalid_chat()) == NULL)
		return NULL;
	
	chat->max_size = max_size;
	
	struct sockaddr_in addr;

	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((chat->epoll = epoll_create1(0)) < 0
		|| (chat->master = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) < 0
		|| (bind(chat->master, (struct sockaddr *)(&addr), sizeof(addr)) < 0) 
		|| (chat->slaves = malloc(sizeof(int) * max_size)) == NULL) {
		
		free_chat(chat);
		return NULL;
	}
		
	return chat; 
}

void free_chat(struct Chat *chat)
{
	if (chat == NULL)
		return;
		
	close(chat->epoll);
	close(chat->master);
	
	for (int i = 0; i < chat->size; i++)
		close(chat->slaves[i]);
	free(chat->slaves);
	
	free(chat);
}

/******************************************************/
/* helper implementation functions not part of an API */
/*                                                    */
/******************************************************/

void broadcast_msg(struct Chat *chat, int fd_from, char *buff, int bytes)
{
	for (int i = 0; i < chat->size; i++) {
		if (chat->slaves[i] == fd_from)
			continue;
		send(chat->slaves[i], buff, bytes, MSG_NOSIGNAL);
	}
}

struct Chat *create_invalid_chat(void)
{
	struct Chat *chat;
	
	if ((chat = malloc(sizeof(struct Chat))) == NULL)
		return NULL;
	
	chat->size = chat->max_size = 0;
	chat->epoll = chat->master = -1;
	chat->slaves = NULL;             

	return chat;
}

int add_client(struct Chat *chat, int client_id)
{
	static struct epoll_event ev;
	
	ev.events  = EPOLLIN;
	ev.data.fd = client_id;

	if (chat->size == chat->max_size
		|| epoll_ctl(chat->epoll, EPOLL_CTL_ADD, client_id, &ev) < 0) 
		return -1;

	chat->slaves[chat->size++] = client_id;
	return 0;
}

int remove_client(struct Chat *chat, int client_id)
{		
	for (int i = 0; i < chat->size; i++) {
		if (chat->slaves[i] != client_id)
			continue;
		chat->slaves[i] = chat->slaves[--chat->size];
		close(client_id);	
		return 0;
	}
	return -1;
}

int set_nonblock(int fd)
{
	int flags;
#ifdef O_NONBLOCK
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	return ioctl(fd, FIONBIO, &flags);
#endif
}
