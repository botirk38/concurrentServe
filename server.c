#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


SSL_CTX* init_ctx(void);
void load_certificates(SSL_CTX* ctx, char* cert_file, char* key_file);
int initialize_server(int port, SSL_CTX **ctx);
void process_json(cJSON *json);
void process_json_value(cJSON *item);

SSL_CTX* init_ctx(void) {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void load_certificates(SSL_CTX* ctx, char* cert_file, char* key_file) {
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        exit(EXIT_FAILURE);
    }
}

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



void parse_request(const char* buffer, char* method, char* uri, int* content_length){
	sscanf(buffer, "%s %s", method, uri);

	const char *content_length_output = strstr(buffer, "Content Length: ");

	if(content_length_output){
		sscanf(content_length_output, "Content-Length: %d", content_length );
	}else{
		*content_length = 0;
	}
}

void parse_headers(const char* buffer, char* content_type, int* content_length) {
    const char *header_line = buffer;

    while (*header_line != '\0' && strncmp(header_line, "\r\n", 2) != 0) {
        if (strncmp(header_line, "Content-Type:", 13) == 0) {
            sscanf(header_line, "Content-Type: %s", content_type);
        } else if (strncmp(header_line, "Content-Length:", 15) == 0) {
            sscanf(header_line, "Content-Length: %d", content_length);
        }
        // Move to the next line
        while (*header_line != '\0' && *header_line != '\n') {
            header_line++;
        }
        if (*header_line == '\n') {
            header_line++;
        }
    }

    if (*content_type == '\0') {
        strcpy(content_type, "text/plain");
    }
}



void process_form_data(const char *data){
	char *token;
	char *rest = strdup(data);

	while((token = strtok_r(rest, "&", &rest))){
		char *key = token;
		char *value = strchr(token, '=');

		if(value){
			*value = '\0';
			value++;

			printf("Key: %s, Value: %s\n",key, value);
		}
	}
}

void process_json_value(cJSON *item){
	switch(item -> type){
		case cJSON_Number:
			printf("Number: %f\n", item -> valuedouble);
			break;
		case cJSON_String:
			printf("String : %s\n", item -> valuestring);
			break;
		case cJSON_Array:
			printf("Array:\n");
			process_json(item -> child);
			break;
		case cJSON_Object:
			printf("Object:\n");
			process_json(item -> child);
			break;
	}
}

void process_json(cJSON *json){
	cJSON *item = json -> child;
	while(item){

		if(item -> string){
			printf("Key: %s\n", item -> string);
		}
		process_json_value(item);
		item = item -> next;
	}
}

void process_put_data(const char *loc, const char *data){
	char filepath[256];
	sprintf(filepath, "data/%s", loc);

	FILE *file = fopen(filepath, "w");

	if(file == NULL){
		perror("Error opening the file");
		return;
	}

	fwrite(data, sizeof(char), strlen(data), file);
	fclose(file);
}

int delete_resource(const char *loc){
	char filepath[256];
	sprintf(filepath, "data/%s", loc);

	if(remove(filepath) == 0){
		printf("File %s deleted successfully.\n", filepath);
		return 0;
	}else{
		printf("File not found.\n");
		return 1;
	}
}


void *handleClient(void* client_socket_ptr){
	int client_socket = *((int *) client_socket_ptr);
	free(client_socket_ptr);
	char buffer[1024] = {0};

	int val_read  = read(client_socket, buffer, 1024);

	if(val_read < 0){
		perror("Error reading from socket.");
	}

	char method[10], uri[50], content_type[50];
	int content_length = 0;
	parse_request(buffer, method, uri, &content_length);
	parse_headers(buffer, content_type, &content_length);

	if(strcmp(method, "GET") == 0 && strcmp(uri, "/") == 0 ){
		char* response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nWelcome to the server!";
		send(client_socket, response, strlen(response), 0);
	} else if(strcmp(method, "POST") == 0 && strcmp(uri, "/") == 0 ){
		char *post_data = malloc(content_length + 1);
		if(!post_data){
			perror("Allocation of memory for post request failed.");
			return NULL;	
		}
		
		read(client_socket, post_data, content_length);
		post_data[content_length - 1] = '\0';

		if(strcmp(content_type,"application/x-www-form-urlencoded") == 0 ){
			process_form_data(post_data);
		} else if(strcmp(content_type, "application/json") == 0){
			cJSON *json_data = cJSON_Parse(post_data);

			if(json_data != NULL){
				process_json(json_data);
				cJSON_Delete(json_data);
			}else{
				perror("JSON data is null");
			}
		}

		free(post_data);

	} else if(strcmp(method, "PUT") == 0 && strcmp(uri, "/") == 0){
		char *put_data = malloc(content_length + 1);

		if(!put_data){
			perror("Failed to allocate memory for put request");
			close(client_socket);
			return NULL;
		}

		read(client_socket, put_data, content_length);
		put_data[content_length] = '\0';

		// Put Data processing
		process_put_data(uri,put_data);	
		free(put_data);

		char* response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nResource updated";	
		} else if(strcmp(method, "DELETE")  == 0 && strcmp(uri, "/") == 0){
			if(delete_resource(uri) == 0){
				char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nResource deleted";
				send(client_socket, response, strlen(response), 0);
			} else {
				char* response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nResource not found";
				send(client_socket, response, strlen(response), 0);
		}
		} else {
			char* response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nResource not found";
			send(client_socket, response, strlen(response), 0);
		}
	return NULL;
}

int main() {
    int port = 8080;
    SSL_CTX *ctx;

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

    // Close the server socket
    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
