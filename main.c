#include "server.h"

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "-p") == 0 && argc > 2) {
            port = atoi(argv[2]);
        }
    }

    /* Create static directory if it doesn't exist */
    mkdir("static", 0755);

    /* Start server */
    start_server(port);

    return 0;
} 