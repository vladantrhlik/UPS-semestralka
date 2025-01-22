#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "server.h"

int main(int argc, char *argv[]) {
	Server s;
	memset(&s, 0, sizeof(Server));

	if (server_create(&s, "config.txt")) return 1;
	for (;;) server_handle(&s);

	return 0;
}
