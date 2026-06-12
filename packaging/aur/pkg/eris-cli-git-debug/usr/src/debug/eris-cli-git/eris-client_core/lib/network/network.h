#ifndef NETWORK_H
#define NETWORK_H

#include <curl/curl.h>

// A wrapper to hold the dynamic string result
typedef struct {
    char *memory;
    size_t size;
} Response;

// Lifecycle
CURL *network_init(void);
void network_cleanup(CURL *curl);

// HTTP Methods
// Returns a Response struct containing the body and size
Response network_get(CURL *curl, const char *url);
Response network_post(CURL *curl, const char *url, const char *data);
Response network_put(CURL *curl, const char *url, const char *data);
Response network_patch(CURL *curl, const char *url, const char *data);
Response network_delete(CURL *curl, const char *url);
Response network_head(CURL *curl, const char *url);
Response network_options(CURL *curl, const char *url);

// Helper to free the response memory when done
void response_free(Response *res);

#endif