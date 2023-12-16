#include "http_utils.h"

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

    if (*header_line == '\0') {
        perror("Failed to parse headers");
    }
}
    // Move to the next line
    