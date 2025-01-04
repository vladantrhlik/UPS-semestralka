#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "server.h"

pthread_mutex_t lock;

int server_ping_loop(Server *s) {
	for (;;) {
		//pthread_mutex_lock(&lock); 
		server_ping(s);
		//pthread_mutex_unlock(&lock); 
	}
	return 0;
}

int server_handle_loop(Server *s) {
	for (;;) {
		//pthread_mutex_lock(&lock); 
		server_handle(s);
		//pthread_mutex_unlock(&lock); 
	}
	return 0;
}

int main(int argc, char *argv[]) {
	Server s;
	pthread_t ping_thread, select_thread;

	memset(&s, 0, sizeof(Server));
	server_create(&s, "config.txt");
	pthread_mutex_init(&lock, NULL);

	//int ping = pthread_create(&ping_thread, NULL, (void*)server_ping_loop, &s);
	int slct = pthread_create(&select_thread, NULL, (void*)server_handle_loop, &s);

	//pthread_join(ping_thread, NULL);
	pthread_join(select_thread, NULL);

	return 0;
}

