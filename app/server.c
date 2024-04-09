#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int handle_client(int client_fd);
char *get_response(char *path);

int handle_client(int client_fd) {
  char buffer[1024];

  ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

  if (bytes_received < 0) {
    printf("Receive failed: %s\n", strerror(errno));
    return 1;
  }
  char method[10], path[100], protocol[10];

  sscanf(buffer, "%s %s %s", method, path, protocol);
  printf("Method: %s\n", method);

  printf("Path: %s\n", path);

  printf("Protocol: %s\n", protocol);

  char *response = get_response(path);

  send(client_fd, response, strlen(response), 0);

  return 0;
}

char *get_response(char *path) {

  if (strcmp(path, "/") == 0) {
    return "HTTP/1.1 200 OK\r\n\r\n";
  } else {
    return "HTTP/1.1 404 Not Found\r\n\r\n";
  }
}

int main() {
  // Disable output buffering
  setbuf(stdout, NULL);

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  printf("Logs from your program will appear here!\n");

  int server_fd;
  socklen_t client_addr_len;
  struct sockaddr_in client_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("Socket creation failed: %s...\n", strerror(errno));
    return 1;
  }

  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
      0) {
    printf("SO_REUSEPORT failed: %s \n", strerror(errno));
    return 1;
  }

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(4221),
      .sin_addr = {htonl(INADDR_ANY)},
  };

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
    printf("Bind failed: %s \n", strerror(errno));
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    printf("Listen failed: %s \n", strerror(errno));
    return 1;
  }

  printf("Waiting for a client to connect...\n");
  client_addr_len = sizeof(client_addr);

  int client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

  if (client_fd < 0) {

    printf("Accept failed: %s \n", strerror(errno));
    return 1;
  }

  int res = handle_client(client_fd);

  if (res != 0) {
    printf("Error in handling client\n");
    return 1;
  }

  close(server_fd);

  return 0;
}

