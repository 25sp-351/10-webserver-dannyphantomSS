#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 4096
#define DEFAULT_PORT 80
#define MAX_THREADS 10

/* Structure to hold request information */
typedef struct {
    char method[10];
    char path[256];
    char headers[MAX_BUFFER_SIZE];
    char body[MAX_BUFFER_SIZE];
} Request;

/* Structure to hold response information */
typedef struct {
    int status_code;
    char content_type[100];
    char body[MAX_BUFFER_SIZE];
    size_t body_length;
} Response;

/* Function declarations */
void start_server(int port);
void* handle_client(void* arg);
void parse_request(char* buffer, Request* req);
void send_response(int client_socket, Response* resp);
void handle_static_request(Request* req, Response* resp);
void handle_calc_request(Request* req, Response* resp);
void handle_sleep_request(Request* req, Response* resp);
char* get_mime_type(const char* filename);
void set_error_response(Response* resp, int status_code, const char* message);

#endif /* SERVER_H */ 