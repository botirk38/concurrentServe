#include "request_handler.h"
#include "file_utils.h"

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

	char method[10], uri[50], content_type[50], filepath[256];
	int content_length = 0;
	const char* contentType = getContentType(uri);
	parse_request(buffer, method, uri, &content_length);

	if(strcmp(method, "GET") == 0){
		sprintf(filepath, "data/%s", uri);

		content_length = getContentLength(filepath);
		if(content_length >=0){
			char response_header[1024];
			sprintf(response_header, 
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n" // Use actual content type
				"Content-Length: %d\r\n"
				"\r\n",
				content_length);
			send(client_socket, response_header, strlen(response_header), 0);
		} else {
			char* response = "HTTP/1.1 404 Not Found\r\n\r\n";
			send(client_socket, response, strlen(response), 0);
		}
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
	} else if(strcmp(method, "HEAD") == 0 && strcmp(uri, "/") == 0) {
		int contentLength = getContentLength(filepath);
		char lastModified[128];
		strncpy(lastModified, getLastModified(uri) ? getLastModified(uri) : "", sizeof(lastModified));
		char headers[1024];
		
		 send(client_socket, headers, strlen(headers), 0); // Won't run yet need to implement getLastModified
		
	} else {
		char* response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nResource not found";
		send(client_socket, response, strlen(response), 0);
	}

	return NULL;
}

