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


int initialize_server(int port);


SSL_CTX* init_ctx(void){
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);

	if(!ctx){
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	reutrn ctx;
}



void load_certificates(SSL_CTX* ctx, char* cert_file, char* key_file) {
    // Set the local certificate from cert_file
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Set the private key from key_file (may be the same as cert_file)
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        exit(EXIT_FAILURE);
    }
}


int initialize_server(int port){
	int server_fd;
	struct sockaddr_in address;

	// Initalize OpenSSL self Certificate
	
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	*ctx = init_ctx();
	load_certificates(*ctx, "path/to/cert.pem", "path/to/key.pem");

	// Create the socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(server_fd == - 1){
		perror("Socket creation failed.");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0){
		perror("Socket binding failed.");
		exit(EXIT_FAILURE);
	}

	if(listen(server_fd, 3) < 0){
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


void parse_headers(const char* buffer, char* content_type){
	const char *content_type_str = strstr(buffer, "Content-Type: ");

	if(content_type_str){
		sscanf(content_type_str, "Content-Type: %s ", content_type);
	}else{
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

void process_json_value(cJSON *json){
	switch(item -> type){
		case cJSON_Number:
			printf("Number: %f\n", item -> valuedouble);
			break;
		case cJSON_String:
			printf("String : %s\n", item -> valuestring);
			break;
		case cJSON_Array:
			printf("Array:\n");
			process_json(item);
			break;
		case cJSON_Object:
			printf("Object:\n");
			process_json(item);
			break;
	}
}

void process_json(cJSON *json){
	cJSON *item = json -> child;
	while(item){

		if(item -> string){
			printf("Key: %s\n", item -> string)
		}
		process_json_value(item);
		item -> item.next;
	}
}


void *handleClient(void* client_socket_ptr){
	int client_socket = *((int *) client_socket_ptr);
	free(client_socket_ptr);
	char buffer[1024] = {0};

	int val_read  = read(client_socket, buffer, 1024);

	if(val_read < 0){
		perror("Error reading from socket.");
		close(client_socket);
		return NULL;
	}
	printf("Received: %s\n", buffer);

	char method[10], uri[50], content_type[50];
	int content_length;
	parse_request(buffer, method, uri, &content_length);
	parse_headers(buffer, content_type);

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
			// Process json data
		}

	} else{
		char* response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nResource not found";
		send(client_socket, response, strlen(response), 0);
	}

	
	return NULL;


}

int main() {
    int port = 8080;
    int server_fd = initialize_server(port);
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
