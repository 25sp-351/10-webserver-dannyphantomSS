#include "server.h"

void start_server(int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;

    /* Create socket file descriptor */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    /* Set socket options */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    /* Bind socket to port */
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    /* Listen for connections */
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        /* Create new thread for client */
        if (thread_count < MAX_THREADS) {
            int* client_socket = malloc(sizeof(int));
            *client_socket = new_socket;
            pthread_create(&threads[thread_count], NULL, handle_client, client_socket);
            thread_count++;
        } else {
            close(new_socket);
        }
    }
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[MAX_BUFFER_SIZE] = {0};
    Request req;
    Response resp;

    /* Read client request */
    if (read(client_socket, buffer, MAX_BUFFER_SIZE) <= 0) {
        close(client_socket);
        return NULL;
    }

    /* Parse request */
    parse_request(buffer, &req);

    /* Handle request based on path */
    if (strncmp(req.path, "/static", 7) == 0) {
        handle_static_request(&req, &resp);
    } else if (strncmp(req.path, "/calc", 5) == 0) {
        handle_calc_request(&req, &resp);
    } else if (strncmp(req.path, "/sleep", 6) == 0) {
        handle_sleep_request(&req, &resp);
    } else {
        set_error_response(&resp, 404, "Not Found");
    }

    /* Send response */
    send_response(client_socket, &resp);
    close(client_socket);
    return NULL;
}

void parse_request(char* buffer, Request* req) {
    char* line = strtok(buffer, "\r\n");
    if (line) {
        sscanf(line, "%s %s", req->method, req->path);
    }
}

void send_response(int client_socket, Response* resp) {
    char response[MAX_BUFFER_SIZE];
    snprintf(response, MAX_BUFFER_SIZE,
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             resp->status_code,
             resp->status_code == 200 ? "OK" : "Not Found",
             resp->content_type,
             resp->body_length,
             resp->body);
    send(client_socket, response, strlen(response), 0);
}

void handle_static_request(Request* req, Response* resp) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "static%s", req->path + 7);
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        set_error_response(resp, 404, "File not found");
        return;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Read file content */
    if (file_size > MAX_BUFFER_SIZE) {
        fclose(file);
        set_error_response(resp, 500, "File too large");
        return;
    }

    fread(resp->body, 1, file_size, file);
    fclose(file);

    resp->body_length = file_size;
    resp->status_code = 200;
    strcpy(resp->content_type, get_mime_type(filepath));
}

void handle_calc_request(Request* req, Response* resp) {
    char operation[10];
    double num1, num2, result;
    char* path = req->path + 6;  /* Skip "/calc/" */

    if (sscanf(path, "%[^/]/%lf/%lf", operation, &num1, &num2) != 3) {
        set_error_response(resp, 400, "Invalid calculation request");
        return;
    }

    if (strcmp(operation, "add") == 0) {
        result = num1 + num2;
    } else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
    } else if (strcmp(operation, "div") == 0) {
        if (num2 == 0) {
            set_error_response(resp, 400, "Division by zero");
            return;
        }
        result = num1 / num2;
    } else {
        set_error_response(resp, 400, "Invalid operation");
        return;
    }

    snprintf(resp->body, MAX_BUFFER_SIZE,
             "<html><body><h1>Calculation Result</h1>"
             "<p>%g %s %g = %g</p></body></html>",
             num1, operation, num2, result);
    resp->body_length = strlen(resp->body);
    resp->status_code = 200;
    strcpy(resp->content_type, "text/html");
}

void handle_sleep_request(Request* req, Response* resp) {
    int seconds;
    if (sscanf(req->path + 7, "%d", &seconds) != 1) {
        set_error_response(resp, 400, "Invalid sleep duration");
        return;
    }

    sleep(seconds);
    snprintf(resp->body, MAX_BUFFER_SIZE, "Slept for %d seconds", seconds);
    resp->body_length = strlen(resp->body);
    resp->status_code = 200;
    strcpy(resp->content_type, "text/plain");
}

char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
        return "text/html";
    if (strcmp(ext, ".txt") == 0)
        return "text/plain";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".gif") == 0)
        return "image/gif";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";

    return "application/octet-stream";
}

void set_error_response(Response* resp, int status_code, const char* message) {
    resp->status_code = status_code;
    snprintf(resp->body, MAX_BUFFER_SIZE, "Error %d: %s", status_code, message);
    resp->body_length = strlen(resp->body);
    strcpy(resp->content_type, "text/plain");
} 