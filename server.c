#include "server.h"


int main() {
    int port = 8080;
    SSL_CTX *ctx;

    int server_fd = initialize_server(port, &ctx);
    printf("Server initialized at port %d\n", port);

    struct sockaddr_in address;
    int addrLen = sizeof(address);

    mongoc_init();
    mongoc_uri_t *uri = mongoc_uri_new("localhost:5000"); 
    mongoc_client_pool_t *pool = mongoc_client_pool_new(uri);

    while (1) {
        int client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrLen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        struct client_handling_args {
            int client_socket;
            mongoc_client_pool_t *mongo_pool;
        };

        struct client_handling_args *args = malloc(sizeof(struct client_handling_args));
        if (!args) {
            perror("Failed to allocate memory for client handling arguments");
            close(client_socket);
            continue;
        }

        args->client_socket = client_socket;
        args->mongo_pool = pool;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handleClient, args) != 0) {
            perror("Failed to create thread");
            close(client_socket);  // Close the client socket as thread creation failed
            free(args);  // Free the allocated memory
            continue;
        }

        pthread_detach(thread_id);  // Detach the thread
    }

    // Cleanup MongoDB resources
    mongoc_client_pool_destroy(pool);
    mongoc_uri_destroy(uri);
    mongoc_cleanup();

    // Close the server socket and cleanup SSL context
    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}

