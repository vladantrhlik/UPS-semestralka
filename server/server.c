#include "server.h"
#include "structs.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int server_create(Server *s, in_addr_t addr, int port) {
	s->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (s->server_socket < 0) {
		printf("Socket create - ERR\n");
		return -1;
	}

	memset(&s->my_addr, 0, sizeof(struct sockaddr_in));
	s->my_addr.sin_family = AF_INET;
	s->my_addr.sin_port = htons(port);
	s->my_addr.sin_addr.s_addr = addr;

	int return_value = bind(s->server_socket, (struct sockaddr *) &s->my_addr, \
		sizeof(struct sockaddr_in));

	if (return_value == 0) 
		printf("Bind - OK\n");
	else {
		printf("Bind - ERR\n");
		return -1;
	}

	return_value = listen(s->server_socket, 5);
	if (return_value == 0){
		printf("Listen - OK\n");
	} else {
		printf("Listen - ER\n");
	}

	// vyprazdnime sadu deskriptoru a vlozime server socket
	FD_ZERO( &s->client_socks );
	FD_SET( s->server_socket, &s->client_socks );

	return 0;
}

int server_handle(Server *s, int (*handle)(Server *s, MsgType type, int fd, char *msg)) {
	static char buffer[BUFFER_SIZE];
	int msg_len;

	s->tests = s->client_socks;
	// sada deskriptoru je po kazdem volani select prepsana sadou deskriptoru kde se neco delo
	int return_value = select( FD_SETSIZE, &s->tests, ( fd_set *)0, ( fd_set *)0, ( struct timeval *)0 );
	if (return_value < 0) {
		printf("Select - ERR\n");
		return -1;
	}

	// vynechavame stdin, stdout, stderr
	for(int fd = 3; fd < FD_SETSIZE; fd++ ){
		// je dany socket v sade fd ze kterych lze cist ?
		if( FD_ISSET( fd, &s->tests ) ){
		// je to server socket ? prijmeme nove spojeni
			if (fd == s->server_socket){
				int client_socket = accept(s->server_socket, (struct sockaddr *) &s->peer_addr, (socklen_t*) &s->len_addr);
				if (client_socket >= 0) {
					FD_SET( client_socket, &s->client_socks );
					//printf("Pripojen novy klient a pridan do sady socketu\n");
					handle(s, CONNECT, client_socket, NULL);
				} else {
					printf("Accept - ERR\n");
				}
			}
			// je to klientsky socket ? prijmem data
			else {
				// pocet bajtu co je pripraveno ke cteni

				ioctl( fd, FIONREAD, &msg_len );
				if (msg_len > BUFFER_SIZE) {
					printf("Zpráva je nějak moc dlouhá\n");
					return -1;
				}
				// mame co cist
				if (msg_len > 0){
					memset(buffer, '\0', 8);
					recv(fd, &buffer, msg_len, 0);
					handle(s, MSG, fd, buffer);
					//printf("[%d]: %s\n", fd, buffer);
				}
				// na socketu se stalo neco spatneho
				else {
					close(fd);
					FD_CLR( fd, &s->client_socks );
					handle(s, DISCONNECT, fd, buffer);
					//printf("Klient se odpojil a byl odebran ze sady socketu\n");
				}
			}
		}
	}

	return 0;
}
