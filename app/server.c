#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 2048
#define METHOD_SIZE 10
#define PATH_SIZE 100
#define PROTOCOL_SIZE 10
#define USER_AGENT_SIZE 100
#define PORT 4221
#define CONNECTION_BACKLOG 5
#define CONTENT_TYPE "Content-Type: text/plain\r\n"

typedef struct {
    char method[METHOD_SIZE];
    char path[PATH_SIZE];
    char protocol[PROTOCOL_SIZE];
    char user_agent[USER_AGENT_SIZE];
} HttpRequest;

int setup_server_socket(int port);
int handle_client(int client_fd);
void parse_request(const char* buffer, HttpRequest* request);
void build_response(const HttpRequest* request, char* response);

int main() {
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
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        if (handle_client(client_fd) != 0) {
            printf("Error handling client\n");
        }

        close(client_fd);
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
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        close(server_fd);
        return -1;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {htonl(INADDR_ANY)},
    };

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0 ||
        listen(server_fd, CONNECTION_BACKLOG) != 0) {
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int handle_client(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
        perror("Receive failed");
        return 1;
    }

    HttpRequest request;
    parse_request(buffer, &request);

    char response[RESPONSE_SIZE];
    build_response(&request, response);

    send(client_fd, response, strlen(response), 0);

    return 0;
}

void parse_request(const char* buffer, HttpRequest* request) {
	printf("Buffer: %s\n", buffer);
    sscanf(buffer, "%s %s %s", request->method, request->path, request->protocol);

    char* ua_start = strstr(buffer, "User-Agent: ");
    if (ua_start) {
        ua_start += 12; // Length of "User-Agent: "
        char* ua_end = strstr(ua_start, "\r\n");
        if (ua_end) {
            size_t len = ua_end - ua_start;
            len = len < sizeof(request->user_agent) - 1 ? len : sizeof(request->user_agent) - 1;
            strncpy(request->user_agent, ua_start, len);
            request->user_agent[len] = '\0';
        }
    }
}

void build_response(const HttpRequest* request, char* response) {
	printf("Request Path: %s\n", request->path);
    if (strcmp(request->path, "/") == 0) {
        // If the root path is requested, return a simple message
        snprintf(response, RESPONSE_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "%sContent-Length: %d\r\n\r\n"
                 "Root path reached",
                 CONTENT_TYPE, (int)strlen("Root path reached"));
    }  else if(strncmp(request -> path, "/echo/", 6) == 0) {

	const char* content = request->path + 6; 
		printf("Content: %s\n", content);
        
        snprintf(response, RESPONSE_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "%sContent-Length: %ld\r\n\r\n" 
                 "%s",
                 CONTENT_TYPE, strlen(content), content);

} else if(strncmp(request -> path, "/user-agent", 11) == 0) {
	// If the user-agent path is requested, return the user agent
	snprintf(response, RESPONSE_SIZE,
		 "HTTP/1.1 200 OK\r\n"
		 "%sContent-Length: %d\r\n\r\n"
		 "%s",
		 CONTENT_TYPE, (int)strlen(request->user_agent), request->user_agent);

	}

	else {
        // For any other path, return a 404 Not Found response
        const char* notFoundBody = "404 Not Found";
        snprintf(response, RESPONSE_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "%sContent-Length: %d\r\n\r\n"
                 "%s",
                 CONTENT_TYPE, (int)strlen(notFoundBody), notFoundBody);
    }

	printf("Response: %s\n", response);
}

