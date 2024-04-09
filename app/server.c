#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int handle_client(int client_fd);
void get_response(char *path, char *response, char *user_agent);
void parse_header(char *buffer, char *method, char *path, char *protocol,
                  char *user_agent);

void parse_header(char *buffer, char *method, char *path, char *protocol,
                  char *user_agent) {
  sscanf(buffer, "%s %s %s", method, path, protocol);

  if (strcmp(path, "/user-agent") == 0) {
    char *start = strstr(buffer, "User-Agent: ");

    if (start != NULL) {

      start += 12;

      char *end = strstr(start, "\r\n");

      ssize_t len = end - start;

      strncpy(user_agent, start, len);

      user_agent[len] = '\0';
      printf("user_agent: %s\n", user_agent);
    }
  }
}

int handle_client(int client_fd) {
  char buffer[1024];

  ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

  if (bytes_received < 0) {
    printf("Receive failed: %s\n", strerror(errno));
    return 1;
  }

  buffer[bytes_received] = '\0';
  printf("Received: %s\n", buffer);
  char method[10], path[100], protocol[10], user_agent[100];

  parse_header(buffer, method, path, protocol, user_agent);

  char response[2048];

  get_response(path, response, user_agent);

  send(client_fd, response, strlen(response), 0);

  return 0;
}

void get_response(char *path, char *response, char *user_agent) {
  char body[1024] = "Not Found";
  char contentType[] = "Content-Type: text/plain\r\n";
  int contentLength = strlen(body);
  if (strcmp(path, "/") == 0) {
    snprintf(response, 2048,
             "HTTP/1.1 200 OK\r\n%sContent-Length: %d\r\n\r\n%s", contentType,
             contentLength, body);
  } else if (strncmp(path, "/echo/", 6) == 0) {
    strcpy(body, path + 6);
    contentLength = strlen(body);

    snprintf(response, 2048,
             "HTTP/1.1 200 OK\r\n%sContent-Length: %d\r\n\r\n%s", contentType,
             contentLength, body);
  } else if (strncmp(path, "/user-agent", 11) == 0) {
    strncpy(body, user_agent, strlen(user_agent));
contentLength = strlen(body);
    snprintf(response, 2048,
             "HTTP/1.1 200 OK\r\n%sContent-Length: %d\r\n\r\n%s", contentType,
             contentLength, body);

    snprintf(response, 2048,
             "HTTP/1.1 200 OK\r\n%sContent-Length: %d\r\n\r\n%s", contentType,
             contentLength, body);
  }

  else {
    snprintf(response, 2048,
             "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
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
  close(client_fd);
  close(server_fd);

  return 0;
}

