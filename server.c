#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>


int initialize_server(int port);



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

	if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0){
		perror("Socket binding failed.");
		exit(EXIT_FAILURE);
	}

	if(listen(server_fd, 3) < 0){
		perror("Listen failed.");
		exit(EXIT_FAILURE);
	}

	return server_fd;
}

void *handleClient(void* client_socket_ptr){
	int client_socket = *((int *) client_socket_ptr);
	free(client_socket_ptr);
	char buffer[1024] = {0};

	int val_read  = read(client_socket, buffer, 1024);

	if(val_read < 0){
		perror("Error reading from socket.");
		close(client_socket);
		return NULL;
	}
	printf("Received: %s\n", buffer);

	char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello from server";
	send(client_socket, response, strlen(response), 0);

	return NULL;


}

int main() {
    int port = 8080;
    int server_fd = initialize_server(port);
    printf("Server initialized at port %d\n", port);

    struct sockaddr_in address;
    int addrLen = sizeof(address);

    while (1) {
        int client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrLen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            perror("Failed to allocate memory for client socket.");
            close(client_socket);  // Close the client socket as we won't be handling it
            continue;
        }

        *client_socket_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handleClient, client_socket_ptr) != 0) {
            perror("Failed to create thread");
            close(client_socket);  // Close the client socket as thread creation failed
            free(client_socket_ptr);  // Free the allocated memory
            continue;
        }

        pthread_detach(thread_id);  // Detach the thread
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
