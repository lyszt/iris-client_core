#include <stdlib.h>
#include <string.h>
#include "network.h"

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Calculate the real size of the data chunk received
    size_t realsize = size * nmemb;
    // Cast the userp to Response struct pointer
    Response *mem = (Response *)userp;

    // Attempt to expand the memory buffer to accommodate new data
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        // Out of memory error
        return 0;
    }

    // Point to the newly allocated memory
    mem->memory = ptr;
    // Copy the received data to the end of existing buffer
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    // Update the total size
    mem->size += realsize;
    // Null-terminate the string
    mem->memory[mem->size] = 0;

    // Return the number of bytes processed
    return realsize;
}

Response network_get(CURL *curl, const char *url) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);
    
    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_post(CURL *curl, const char *url, const char *data) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set POST method
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    // Set the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_put(CURL *curl, const char *url, const char *data) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set custom request method to PUT
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    // Set the PUT data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_patch(CURL *curl, const char *url, const char *data) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set custom request method to PATCH
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    // Set the PATCH data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_delete(CURL *curl, const char *url) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set custom request method to DELETE
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_head(CURL *curl, const char *url) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Enable HEAD request (no body in response)
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}

Response network_options(CURL *curl, const char *url) {
    // Initialize response struct with zero values
    Response chunk = {0};
    // Allocate initial buffer
    chunk.memory = malloc(1);
    // Start with size 0
    chunk.size = 0;

    // Set the target URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set custom request method to OPTIONS
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    // Set the callback function to handle received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    // Pass the chunk struct as user data to the callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Execute the HTTP request
    CURLcode res = curl_easy_perform(curl);

    // Check if the request failed
    if(res != CURLE_OK) {
        // Print error message to stderr
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Return the response containing the fetched data and its size
    return chunk;
}