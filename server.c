#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "ssl_utils.h"
#include "networking.h"
#include "request_handler.h"
#include "http_utils.h"
#include <mongoc.h>

int main() {
    int port = 8080;
    SSL_CTX *ctx;
    mongoc_client_t *mongo_client;
    mongo_uri_t *uri;
    mongoc_client_pool_t *pool;

    int server_fd = initialize_server(port, &ctx);
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
    mongoc_client_pool_push(pool, mongo_client);
    mongoc_client_pool_destroy(pool);
    mongoc_uri_destroy(uri);
    mongoc_cleanup();
    // Close the server socket
    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
