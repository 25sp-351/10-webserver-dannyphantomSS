# HTTP Server in C

A multithreaded HTTP/1.1 server implementation in C.

## Features

- Multithreaded request handling
- Support for static file serving
- Calculator endpoint with basic arithmetic operations
- Sleep endpoint for testing
- Proper HTTP/1.1 response formatting
- Content-Type and Content-Length headers

## Requirements

- GCC compiler
- Make
- POSIX-compliant operating system

## Building

To build the server:

```bash
make
```

## Usage

Start the server with default port (80):
```bash
./http_server
```

Start the server with a custom port:
```bash
./http_server -p 8080
```

## Endpoints

### Static Files
- URL: `/static/<path>`
- Example: `/static/images/example.png`
- Serves files from the `static` directory

### Calculator
- URL: `/calc/<operation>/<num1>/<num2>`
- Operations: add, mul, div
- Example: `/calc/add/5/3`

### Sleep
- URL: `/sleep/<seconds>`
- Example: `/sleep/5`

## Testing

You can test the server using:
1. Web browser
2. Telnet
3. Postman or Thunder Client

### Example Requests

1. Calculator:
```
GET /calc/add/5/3 HTTP/1.1
Host: localhost:80
```

2. Static File:
```
GET /static/images/example.png HTTP/1.1
Host: localhost:80
```

3. Sleep:
```
GET /sleep/5 HTTP/1.1
Host: localhost:80
```

## Directory Structure

```
.
├── main.c
├── server.c
├── server.h
├── Makefile
├── README.md
└── static/
    └── images/
        └── example.txt
```

