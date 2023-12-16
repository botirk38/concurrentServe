
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ssl_utils.h"
#include "networking.h"

// Initializes the server socket and sets up SSL context
int initialize_server(int port, SSL_CTX **ctx) {
    int server_fd;
    struct sockaddr_in address;

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    *ctx = init_ctx();
    load_certificates(*ctx, "path/to/cert.pem", "path/to/key.pem");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket binding failed.");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed.");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}



