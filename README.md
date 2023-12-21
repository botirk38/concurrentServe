
# ConcurrentServe

## Description
This project is a concurrent server built in C, designed to handle multiple client requests simultaneously. It integrates MongoDB for data storage, allowing CRUD (Create, Read, Update, Delete) operations on a MongoDB database. The server uses OpenSSL for SSL encryption, ensuring secure data transmission.

## Features
- Concurrent handling of client requests using threads.
- SSL encryption for secure communication.
- Integration with MongoDB for persistent data storage.
- Supports CRUD operations via HTTP methods: GET, POST, PUT, DELETE.
- JSON parsing for request and response handling.
- Modularized code structure for easy maintenance and scalability.

## Installation

### Prerequisites
- C compiler (GCC or Clang)
- OpenSSL
- MongoDB C Driver
- cJSON library

### Steps
1. Clone the repository:
   ```bash
   git clone [https://github.com/botirk38/concurrentServe]
   ```

2. Navigate to the project directory:
   ```bash
   cd [server]
   ```

3. Compile the server:
   ```bash
   make
   ```

## Usage
To start the server, run the compiled executable:
```bash
./server
```

The server will start listening for client connections on the specified port.

## API Endpoints
The server supports the following endpoints:
- `GET /resource`: Fetches data from the MongoDB database.
- `POST /resource`: Creates new data in the MongoDB database.
- `PUT /resource`: Updates existing data in the MongoDB database.
- `DELETE /resource`: Deletes data from the MongoDB database.

## Contributing
Contributions to this project are welcome. Please fork the repository and submit a pull request with your changes.

## License
This project is licensed under the [MIT License](LICENSE).

## Contact
For any queries or contributions, please contact [btrghstk@gmail.com].

