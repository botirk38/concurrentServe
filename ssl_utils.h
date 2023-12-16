// ssl_utils.h
#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* init_ctx(void);
void load_certificates(SSL_CTX* ctx, char* cert_file, char* key_file);

#endif
