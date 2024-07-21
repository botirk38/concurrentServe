#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>

#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 2048
#define METHOD_SIZE 10
#define PATH_SIZE 100
#define PROTOCOL_SIZE 10
#define USER_AGENT_SIZE 100
#define ACCEPT_ENCODING_SIZE 100
#define CONTENT_ENCODING_SIZE 100
#define ACCEPT_ENCODING_SIZE 100

#define PORT 4221
#define CONNECTION_BACKLOG 5
#define CONTENT_TYPE "Content-Type: text/plain\r\n"
#define CONTENT_TYPE_FILE "Content-Type: application/octet-stream\r\n"
#define BODY_SIZE 1024

typedef struct {
  char method[METHOD_SIZE];
  char path[PATH_SIZE];
  char protocol[PROTOCOL_SIZE];
  char user_agent[USER_AGENT_SIZE];
  char content_encoding[CONTENT_ENCODING_SIZE];
  char accept_encoding[ACCEPT_ENCODING_SIZE];
  int content_length;
  char body[BODY_SIZE];
} HttpRequest;

int setup_server_socket(int port);
void *handle_client(void *arg);
void parse_request(const char *buffer, HttpRequest *request);
void build_response(const HttpRequest *request, char *response, int client_fd);
void parse_cli_args(int argc, char **argv);
void send_headers(int client_fd, const char *status, const char *content_type,
                  long content_length, const char *content_encoding,
                  const char *accept_encoding);
void send_not_found(int client_fd);
void send_text_response(int client_fd, const char *status,
                        const char *content_type, const char *body,
                        const char *accept_encoding);
void handle_post_request(const HttpRequest *request, int client_fd);
void handle_get_request(const HttpRequest *request, int client_fd);
int gzip_compress(const char *src, int srcLen, char *dst, int dstLen);

int gzip_compress(const char *src, int srcLen, char *dst, int dstLen) {
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    return -1;
  }

  strm.avail_in = srcLen;
  strm.next_in = (Bytef *)src;
  strm.avail_out = dstLen;
  strm.next_out = (Bytef *)dst;

  int ret = deflate(&strm, Z_FINISH);
  deflateEnd(&strm);

  if (ret != Z_STREAM_END) {
    return -1;
  }

  return dstLen - strm.avail_out;
}

char *directory = NULL;

int main(int argc, char **argv) {

  parse_cli_args(argc, argv);

  setbuf(stdout, NULL);
  printf("Server starting...\n");

  int server_fd = setup_server_socket(PORT);
  if (server_fd < 0) {
    perror("Failed to set up the server socket");
    return 1;
  }

  while (1) {
    printf("Waiting for a client to connect...\n");
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
      close(*client_fd); // Close the client socket if thread creation fails
      free(client_fd);   // Free the dynamically allocated memory
    } else {
      pthread_detach(thread_id); // Detach the thread
    }
  }

  close(server_fd);
  return 0;
}

int setup_server_socket(int port) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    return -1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
      0) {
    close(server_fd);
    return -1;
  }

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = {htonl(INADDR_ANY)},
  };

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0 ||
      listen(server_fd, CONNECTION_BACKLOG) != 0) {
    close(server_fd);
    return -1;
  }

  return server_fd;
}

void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);
  char buffer[BUFFER_SIZE] = {0};
  ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  if (bytes_received < 0) {
    perror("Receive failed");
    return NULL;
  }

  HttpRequest request;
  parse_request(buffer, &request);

  char response[RESPONSE_SIZE];
  build_response(&request, response, client_fd);
  close(client_fd);

  return NULL;
}

void parse_request(const char *buffer, HttpRequest *request) {
  sscanf(buffer, "%s %s %s", request->method, request->path, request->protocol);

  char *content_length_start = strstr(buffer, "Content-Length: ");

  if (content_length_start) {
    content_length_start += 16;

    request->content_length = atoi(content_length_start);
  }
  char *ua_start = strstr(buffer, "User-Agent: ");
  if (ua_start) {
    ua_start += 12; // Length of "User-Agent: "
    char *ua_end = strstr(ua_start, "\r\n");
    if (ua_end) {
      size_t len = ua_end - ua_start;
      len = len < sizeof(request->user_agent) - 1
                ? len
                : sizeof(request->user_agent) - 1;
      strncpy(request->user_agent, ua_start, len);
      request->user_agent[len] = '\0';
    }
  }

  char *content_encoding_start = strstr(buffer, "Content-Encoding: ");

  if (content_encoding_start) {

    content_encoding_start += 17; // Length of "Content-Encoding: "

    char *content_encoding_end = strstr(content_encoding_start, "\r\n");
    size_t len = content_encoding_end - content_encoding_start;

    if (content_encoding_end) {

      strncpy(request->user_agent, content_encoding_start, len);
      request->content_encoding[len] = '\0';
    }
  }

  char *accept_encoding_start = strstr(buffer, "Accept-Encoding: ");
  if (accept_encoding_start) {
    accept_encoding_start += 17; // Length of "Accept-Encoding: "
    char *accept_encoding_end = strstr(accept_encoding_start, "\r\n");
    if (accept_encoding_end) {
      size_t len = accept_encoding_end - accept_encoding_start;
      len = len < sizeof(request->accept_encoding) - 1
                ? len
                : sizeof(request->accept_encoding) - 1;
      strncpy(request->accept_encoding, accept_encoding_start, len);
      request->accept_encoding[len] = '\0';
    }
  }

  char *body_start = strstr(buffer, "\r\n\r\n");
  if (body_start) {
    body_start += 4;
    size_t len = strlen(buffer) - (body_start - buffer);
    len = len < sizeof(request->body) - 1 ? len : sizeof(request->body) - 1;
    strncpy(request->body, body_start, len);
    request->body[len] = '\0';
  }
}

void build_response(const HttpRequest *request, char *response, int client_fd) {
  printf("Request Path: %s\n", request->path);
  if (strcmp(request->method, "GET") == 0) {
    handle_get_request(request, client_fd);
  } else if (strncmp(request->method, "POST", 4) == 0) {
    handle_post_request(request, client_fd);
  }
}

void parse_cli_args(int argc, char **argv) {

  if (argc != 3) {
    fprintf(stderr, "Usage: %s --directory <directory>\n", argv[0]);
  }

  directory = argv[2];
  printf("Directory: %s\n", directory);
}

void send_headers(int client_fd, const char *status, const char *content_type,
                  long content_length, const char *accept_encoding,
                  const char *content_encoding) {
  char header[RESPONSE_SIZE];
  const char *encoding_header = "";

  if (accept_encoding && strstr(accept_encoding, "gzip")) {
    encoding_header = "Content-Encoding: gzip\r\n";
  }

  snprintf(header, sizeof(header),
           "%s\r\n"
           "%s"
           "%s"
           "Content-Length: %ld\r\n\r\n",
           status, content_type, encoding_header, content_length);
  send(client_fd, header, strlen(header), 0);
}
void send_not_found(int client_fd) {
  const char *body = "404 Not Found";
  send_text_response(client_fd, "HTTP/1.1 404 Not Found", CONTENT_TYPE, body,
                     NULL);
}

void send_text_response(int client_fd, const char *status,
                        const char *content_type, const char *body,
                        const char *accept_encoding) {
  if (accept_encoding && strstr(accept_encoding, "gzip")) {
    char compressed[BUFFER_SIZE];
    int body_length = strlen(body);
    int compressed_length =
        gzip_compress(body, body_length, compressed, BUFFER_SIZE);

    if (compressed_length > 0) {
      char headers[BUFFER_SIZE];
      snprintf(headers, BUFFER_SIZE,
               "%s\r\n"
               "%s"
               "Content-Encoding: gzip\r\n"
               "Content-Length: %d\r\n\r\n",
               status, content_type, compressed_length);

      send(client_fd, headers, strlen(headers), 0);
      send(client_fd, compressed, compressed_length, 0);
    } else {
      // Fallback to uncompressed if compression fails
      send_headers(client_fd, status, content_type, strlen(body), NULL, NULL);
      send(client_fd, body, strlen(body), 0);
    }
  } else {
    send_headers(client_fd, status, content_type, strlen(body), NULL, NULL);
    send(client_fd, body, strlen(body), 0);
  }
}

void handle_post_request(const HttpRequest *request, int client_fd) {
  if (strncmp(request->path, "/files/", 7) == 0) {
    char file_path[PATH_SIZE];
    snprintf(file_path, sizeof(file_path), "%s%s", directory,
             request->path + 7);

    // Simple path sanitation
    if (strstr(file_path, "..")) {
      send_not_found(client_fd);
      return;
    }

    FILE *file = fopen(file_path, "wb");
    if (file) {
      fwrite(request->body, 1, request->content_length, file);
      fclose(file);

      // Successful file creation response
      const char *message = "File uploaded successfully";
      send_text_response(client_fd, "HTTP/1.1 201 Created", CONTENT_TYPE,
                         message, request->accept_encoding);
    } else {
      send_not_found(client_fd);
    }
  } else {
    send_not_found(client_fd);
  }
}

void handle_get_request(const HttpRequest *request, int client_fd) {
  // Root path response
  if (strcmp(request->path, "/") == 0) {
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE,
                       "Root path reached", request->accept_encoding);
  }
  // Echo response
  else if (strncmp(request->path, "/echo/", 6) == 0) {
    const char *content = request->path + 6;
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE, content,
                       request->accept_encoding);
  }
  // User-Agent response
  else if (strncmp(request->path, "/user-agent", 11) == 0) {
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE,
                       request->user_agent, request->accept_encoding);
  }
  // File serving from "/files/" path
  else if (strncmp(request->path, "/files/", 7) == 0) {
    char file_path[PATH_SIZE];
    snprintf(file_path, sizeof(file_path), "%s%s", directory,
             request->path + 7);

    // Security: Sanitize file_path to prevent directory traversal attacks
    if (strstr(file_path, "..")) {
      send_not_found(client_fd);
      return;
    }

    FILE *file = fopen(file_path, "rb");
    if (file) {
      fseek(file, 0, SEEK_END);
      long file_size = ftell(file);
      fseek(file, 0, SEEK_SET);

      // Allocate memory for reading file content
      char *file_content = malloc(file_size);
      if (!file_content) {
        perror("Failed to allocate memory for file content");
        fclose(file);
        return;
      }

      fread(file_content, 1, file_size, file);
      fclose(file);

      // Send file content
      send_headers(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE_FILE, file_size,
                   request->accept_encoding, request->content_encoding);
      send(client_fd, file_content, file_size, 0); // Actual file content

      free(file_content);
    } else {
      send_not_found(client_fd);
    }
  } else {
    // If no route matched, send 404 Not Found
    send_not_found(client_fd);
  }
}
