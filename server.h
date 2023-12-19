#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "ssl_utils.h"
#include "networking.h"
#include "request_handler.h"
#include "http_utils.h"
#include <mongoc.h>


int main();

#endif
