#ifndef HTTP_H
#define HTTP_H

#define PORT 4221

typedef struct {
  char method[10];
  char path[100];
  char protocol[10];
  char user_agent[100];
  char content_encoding[100];
  char accept_encoding[100];
  int content_length;
  char body[1024];
} HttpRequest;

int setup_server_socket(int port);
void *handle_client(void *arg);
void parse_request(const char *buffer, HttpRequest *request);
void build_response(const HttpRequest *request, int client_fd);

#endif
