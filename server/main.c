#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "server.h"
#include "structs.h"

int handle_msg(MsgType type, int fd, char *msg) {
	switch (type) {
		case CONNECT:
			printf("New client: %d\n", fd);
		break;
		case MSG:
			printf("[%d]: %s", fd, msg);
		break;
		case DISCONNECT:
			printf("Disconnected: %d\n", fd);
		break;
	}
	return 0;
}

int main (void){
	Server s;

	server_create(&s, INADDR_ANY, 10000);
	for (;;) server_handle(&s, &handle_msg);

	return 0;
}
