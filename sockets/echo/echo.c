#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int init_tcp_socket(const char *addr, uint16_t port);
void serve(int s);

int main(int argc, char **argv)
{
	if (argc < 2)
		return -1;
	uint16_t port;
	if (sscanf(argv[1], "%hu", &port) != 1)
		return -1;
	int s = init_tcp_socket("127.0.0.1", port);
	if (s < 0)
		return -1;
	serve(s);
	close(s);
	return 0;
}

void echo_requests(int cs);

void serve(int s)
{
	int cs;
	listen(s, SOMAXCONN);
	do {
		if ((cs = accept(s, NULL, NULL)) < 0)
			continue;
		echo_requests(cs);
		close(cs);
	} while (1);
}

void echo_requests(int cs)
{
	int bytes;
	char buf[BUFSIZ];
	do {
		bytes = recv(cs, buf, BUFSIZ - 1, 0);
		if (bytes <= 0 || strncmp(buf, "OFF", 3) == 0)
			break;
		send(cs, buf, bytes, MSG_NOSIGNAL);
	} while (1);
}

int init_tcp_socket(const char *addr, uint16_t port)
{
	int family = AF_INET;
	int s = socket(family, SOCK_STREAM, 0);
	if (s < 0)
		return -1;
	struct sockaddr_in in_addr;
	memset(&in_addr, 0, sizeof(struct sockaddr_in));
	in_addr.sin_family = family;
	in_addr.sin_port = htons(port);
	if (inet_pton(family, addr, &in_addr.sin_addr) <= 0)
		return -1;
	if (bind(s, (struct sockaddr *) &in_addr, sizeof(struct sockaddr_in)) < 0)
		return -1;
	return s;
}
