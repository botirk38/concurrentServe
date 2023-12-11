# concurrentServe

## Overview

ConcurrentServe is a compact, multithreaded web server in C, designed for educational purposes. It demonstrates TCP/IP networking, thread management, and HTTP protocol handling, offering a practical approach to learning low-level network programming.

## Features
- Basic TCP socket server setup.
- Multithreading to handle multiple client connections.
- HTTP request parsing and response handling.
- Serving static content from the server's filesystem.

## Requirements
- GCC Compiler
- POSIX-compliant operating system (Linux, Unix)
- Basic knowledge of C programming and networking

## Installation
Clone the repository to your local machine:
```sh
git clone [https://github.com/yourusername/multithreaded-webserver-c.git](https://github.com/botirk38/concurrentServe/tree/main)
cd concurrentServe
```

## Compilation
To compile the server, run:
```sh
gcc -o server server.c -lpthread
```

## Usage
To start the server, run:
```sh
./server
```
By default, the server listens on port 8080. To access the web server, navigate to `http://localhost:8080` in your web browser.

## Configuration
- **Port Configuration**: The default port is set to 8080. This can be changed in the server source code.
- **Static Content**: Place your static files in the designated server directory.

## Contributing
We welcome contributions! Please submit pull requests, report bugs, and suggest features.

### Pull Requests
1. Fork the repository.
2. Create a new branch (`git checkout -b feature-branch`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature-branch`).
5. Create a new Pull Request.

## License
This project is licensed under the [MIT License](LICENSE).

## Acknowledgments
- Contributors and community members
- Any third-party assets or libraries used


### Changelog for v1.0

#### Version: 1.0
_Date: 11/12/2023

#### New Features:
1. **Basic TCP Server Setup**
   - Implemented a TCP server that can listen to and accept incoming connections on a specified port.
   - Used socket programming to create, bind, and listen on a server socket.

2. **Multithreaded Client Handling**
   - Integrated multithreading to handle multiple client connections simultaneously.
   - Each client connection is handled in a separate thread to ensure non-blocking server operations.

3. **HTTP Request Parsing**
   - Added functionality to parse HTTP requests from clients.
   - The server can differentiate between `GET` and `POST` methods and extract the URI from the request line.

4. **Handling GET Requests**
   - Implemented the ability to respond to `GET` requests.
   - The server sends a welcoming text response for a `GET` request to the root path (`/`).

5. **Dynamic Response for Not Found Paths**
   - Added handling for unrecognized paths with a 404 Not Found response.

6. **Form Data Processing for POST Requests**
   - Implemented processing of `application/x-www-form-urlencoded` form data in POST requests.
   - The server can parse key-value pairs from the POST request body and print them to the console.

7. **Basic Request Header Parsing**
   - Enhanced request parsing to include extracting the `Content-Length` and `Content-Type` headers.

8. **Robust Error Handling and Logging**
   - Improved error handling across server operations, including socket creation, binding, listening, and client handling.
   - Added logging for key events and errors.

#### Improvements:
1. **Refactored Code for Clarity and Modularity**
   - Organized the server code into functions for better readability and maintainability.
   - Separated concerns such as server initialization, client handling, and request parsing.

2. **Enhanced Security Measures**
   - Input validation and sanitization were added to handle client data more securely.
   - Prepared the server for future integration of security features like HTTPS.

3. **Performance Optimizations**
   - Optimized multithreading usage to ensure efficient handling of multiple simultaneous connections.

#### Notes for Future Development:
- The server is currently in a basic state, ideal for educational purposes and a foundational understanding of web server operations.
- Further development, particularly for production use, would require additional security measures, performance tuning, and expanded HTTP request handling capabilities.
- JSON parsing and processing is planned for the next feature branch to enhance the server's capabilities in handling modern web data formats.

