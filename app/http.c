#include "http.h"
#include "gzip.h"
#include "server.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 2048
#define CONTENT_TYPE "Content-Type: text/plain\r\n"
#define CONTENT_TYPE_FILE "Content-Type: application/octet-stream\r\n"
#define BODY_SIZE 1024

// Forward declarations for internal helpers
static void send_headers(int client_fd, const char *status,
                         const char *content_type, long content_length,
                         const char *accept_encoding,
                         const char *content_encoding, const char *connection);
static void send_not_found(int client_fd, const char *connection);
static void send_text_response(int client_fd, const char *status,
                               const char *content_type, const char *body,
                               const char *accept_encoding,
                               const char *connection);
static void handle_post_request(const HttpRequest *request, int client_fd,
                                const char *connection);
static void handle_get_request(const HttpRequest *request, int client_fd,
                               const char *connection);

// Set up the server socket
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
      listen(server_fd, 5) != 0) {
    close(server_fd);
    return -1;
  }

  return server_fd;
}

// Thread entry point for handling a client
void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);

  while (1) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
      perror("Receive failed");
      break;
    }
    if (bytes_received == 0) {
      // Client closed connection
      break;
    }

    HttpRequest request;
    memset(&request, 0, sizeof(HttpRequest));
    parse_request(buffer, &request);

    // Detect if the client wants to close the connection
    int close_connection = 0;
    if (strstr(buffer, "Connection: close")) {
      close_connection = 1;
    }

    // Pass the connection header to the response
    const char *connection_header =
        close_connection ? "Connection: close\r\n" : NULL;

    build_response(&request, client_fd, connection_header);

    if (close_connection) {
      break;
    }
  }
  close(client_fd);
  return NULL;
}

// Parse the HTTP request from the buffer
void parse_request(const char *buffer, HttpRequest *request) {
  sscanf(buffer, "%s %s %s", request->method, request->path, request->protocol);

  char *content_length_start = strstr(buffer, "Content-Length: ");
  if (content_length_start) {
    content_length_start += 16;
    request->content_length = atoi(content_length_start);
  }

  char *ua_start = strstr(buffer, "User-Agent: ");
  if (ua_start) {
    ua_start += 12;
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
    content_encoding_start += 17;
    char *content_encoding_end = strstr(content_encoding_start, "\r\n");
    if (content_encoding_end) {
      size_t len = content_encoding_end - content_encoding_start;
      len = len < sizeof(request->content_encoding) - 1
                ? len
                : sizeof(request->content_encoding) - 1;
      strncpy(request->content_encoding, content_encoding_start, len);
      request->content_encoding[len] = '\0';
    }
  }

  char *accept_encoding_start = strstr(buffer, "Accept-Encoding: ");
  if (accept_encoding_start) {
    accept_encoding_start += 17;
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

// Build and send the HTTP response
void build_response(const HttpRequest *request, int client_fd,
                    const char *connection) {
  if (strcmp(request->method, "GET") == 0) {
    handle_get_request(request, client_fd, connection);
  } else if (strcmp(request->method, "POST") == 0) {
    handle_post_request(request, client_fd, connection);
  } else {
    send_not_found(client_fd, connection);
  }
}

// Send HTTP headers
static void send_headers(int client_fd, const char *status,
                         const char *content_type, long content_length,
                         const char *accept_encoding,
                         const char *content_encoding, const char *connection) {
  char header[RESPONSE_SIZE];
  const char *encoding_header = "";

  if (accept_encoding && strstr(accept_encoding, "gzip")) {
    encoding_header = "Content-Encoding: gzip\r\n";
  }

  snprintf(header, sizeof(header),
           "%s\r\n"
           "%s"
           "%s"
           "%s"
           "Content-Length: %ld\r\n\r\n",
           status, content_type, encoding_header,
           (connection ? connection : ""), content_length);

  send(client_fd, header, strlen(header), 0);
}

// Send a 404 Not Found response
static void send_not_found(int client_fd, const char *connection) {
  const char *body = "404 Not Found";
  send_text_response(client_fd, "HTTP/1.1 404 Not Found", CONTENT_TYPE, body,
                     NULL, connection);
}

// Send a text response, optionally gzipped
static void send_text_response(int client_fd, const char *status,
                               const char *content_type, const char *body,
                               const char *accept_encoding,
                               const char *connection) {
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
               "%s"
               "Content-Length: %d\r\n\r\n",
               status, content_type, (connection ? connection : ""),
               compressed_length);

      send(client_fd, headers, strlen(headers), 0);
      send(client_fd, compressed, compressed_length, 0);
    } else {
      // Fallback to uncompressed if compression fails
      send_headers(client_fd, status, content_type, strlen(body), NULL, NULL,
                   connection);
      send(client_fd, body, strlen(body), 0);
    }
  } else {
    send_headers(client_fd, status, content_type, strlen(body), NULL, NULL,
                 connection);
    send(client_fd, body, strlen(body), 0);
  }
}

// Handle POST requests (file upload)
static void handle_post_request(const HttpRequest *request, int client_fd,
                                const char *connection) {
  if (strncmp(request->path, "/files/", 7) == 0) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s%s", directory,
             request->path + 7);

    // Simple path sanitation
    if (strstr(file_path, "..")) {
      send_not_found(client_fd, connection);
      return;
    }

    FILE *file = fopen(file_path, "wb");
    if (file) {
      fwrite(request->body, 1, request->content_length, file);
      fclose(file);

      const char *message = "File uploaded successfully";
      send_text_response(client_fd, "HTTP/1.1 201 Created", CONTENT_TYPE,
                         message, request->accept_encoding, connection);
    } else {
      send_not_found(client_fd, connection);
    }
  } else {
    send_not_found(client_fd, connection);
  }
}

// Handle GET requests (root, echo, user-agent, file download)
static void handle_get_request(const HttpRequest *request, int client_fd,
                               const char *connection) {
  // Root path response
  if (strcmp(request->path, "/") == 0) {
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE,
                       "Root path reached", request->accept_encoding,
                       connection);
  }
  // Echo response
  else if (strncmp(request->path, "/echo/", 6) == 0) {
    const char *content = request->path + 6;
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE, content,
                       request->accept_encoding, connection);
  }
  // User-Agent response
  else if (strncmp(request->path, "/user-agent", 11) == 0) {
    send_text_response(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE,
                       request->user_agent, request->accept_encoding,
                       connection);
  }
  // File serving from "/files/" path
  else if (strncmp(request->path, "/files/", 7) == 0) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s%s", directory,
             request->path + 7);

    // Security: Sanitize file_path to prevent directory traversal attacks
    if (strstr(file_path, "..")) {
      send_not_found(client_fd, connection);
      return;
    }

    FILE *file = fopen(file_path, "rb");
    if (file) {
      fseek(file, 0, SEEK_END);
      long file_size = ftell(file);
      fseek(file, 0, SEEK_SET);

      char *file_content = malloc(file_size);
      if (!file_content) {
        perror("Failed to allocate memory for file content");
        fclose(file);
        send_not_found(client_fd, connection);
        return;
      }

      fread(file_content, 1, file_size, file);
      fclose(file);

      send_headers(client_fd, "HTTP/1.1 200 OK", CONTENT_TYPE_FILE, file_size,
                   request->accept_encoding, request->content_encoding,
                   connection);
      send(client_fd, file_content, file_size, 0);

      free(file_content);
    } else {
      send_not_found(client_fd, connection);
    }
  } else {
    send_not_found(client_fd, connection);
  }
}
