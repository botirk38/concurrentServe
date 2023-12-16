#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <openssl/ssl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h> 
#include "request_handler.h"
#include "file_utils.h"
#include "http_utils.h"

void *handleClient(void* client_socket_ptr);
void process_json(cJSON *json);
void process_json_value(cJSON *item);
void process_put_data(const char *loc, const char *data);
int delete_resource(const char *loc);

#endif // REQUEST_HANDLER_H
