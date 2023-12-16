#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <string.h>
#include <stdio.h>

void parse_headers(const char* buffer, char* content_type, int* content_length);
void parse_request(const char* buffer, char* method, char* uri, int* content_length);

#endif
