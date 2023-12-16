#include "networking.h"
#include "file_utils.h"


bool endsWith(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr) {
        return false;
    }
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

const char* getContentType(const char* uri) {

    if (endsWith(uri, ".html") || endsWith(uri, ".htm")) {
        return "text/html";
    } else if (endsWith(uri, ".jpg") || endsWith(uri, ".jpeg")) {
        return "image/jpeg";
    } else if (endsWith(uri, ".png")) {
        return "image/png";
    } else if (endsWith(uri, ".css")) {
        return "text/css";
    } else if (endsWith(uri, ".js")) {
        return "application/javascript";
    } else if (endsWith(uri, ".json")) {
        return "application/json";
    } else {
        // Default content type
        return "text/plain";
    }
}

const char* getLastModified(const char* uri){
	struct stat statbuf;
	static char lastModified[128];
	char filepath[256];
	sprintf(filepath, "data%s", uri);

	if(stat(filepath, &statbuf) == 0){
		struct tm *tm = gmtime(&statbuf.st_mtime);
		strftime(lastModified, sizeof(lastModified), "%a, %d %b %Y %H:%M:%S GMT", tm);
		return lastModified;
	} else {
		perror("Failed to get last modified time");
		return NULL;
	}



}

const int getContentLength(const char* file_path){
	struct stat file_stat;

	if(stat(file_path, &file_stat) == 0){
		return file_stat.st_size;
	}

	return -1;
}
