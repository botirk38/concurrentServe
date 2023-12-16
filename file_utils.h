#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>
#include <time.h>  
#include <stdio.h>


bool endsWith(const char* str, const char *suffix);
const char* getContentType(const char* uri);
const char* getLastModified(const char* uri);
const int getContentLength(const char* file_path);

#endif // FILE_UTILS_H