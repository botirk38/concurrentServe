#include "server.h"
#include "http.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *directory = NULL;

void parse_cli_args(int argc, char **argv) {
  if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
    directory = argv[2];
  } else if (argc == 1) {
    // No directory specified, use current directory
    directory = "./";
  } else {
    fprintf(stderr, "Usage: %s --directory <directory>\n", argv[0]);
    exit(1);
  }
  printf("Directory: %s\n", directory);
}

void start_server() {
  int server_fd = setup_server_socket(PORT);
  if (server_fd < 0) {
    perror("Failed to set up the server socket");
    exit(1);
  }
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));
    *client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (*client_fd < 0) {
      perror("Accept failed");
      free(client_fd);
      continue;
    }
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
      perror("Failed to create thread");
      close(*client_fd);
      free(client_fd);
    } else {
      pthread_detach(thread_id);
    }
  }
  close(server_fd);
}
