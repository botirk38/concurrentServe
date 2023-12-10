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
git clone https://github.com/yourusername/multithreaded-webserver-c.git
cd multithreaded-webserver-c
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
We welcome contributions! Please feel free to submit pull requests, report bugs, and suggest features.

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
