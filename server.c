#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

initialize_server(int port);
create_socket();


int initialize_server(int port){
	int server_fd;
	struct sockaddr_in address;

	// Create the socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(server_fd == - 1){
		perror("Socket creation failed.");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if(bind(server_fd, (*struct sockaddr) &address, sizeof(address)) < 0){
		perror("Socket binding failed.")
	}
}
