#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "server.h"
#include "structs.h"
#include "handlers.h"
#include "utils.h"


int main(int argc, char *argv[]) {
	Server s;
	memset(&s, 0, sizeof(Server));

	server_create(&s, "config.txt");
	for (;;) server_handle(&s);

	return 0;
}
