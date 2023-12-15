

### Changelog for Release v1.5.0 - 15/12/2023

#### New Features:
- **JSON Parsing Implementation:** 
  - Added support for parsing JSON in POST requests. This feature allows the server to handle incoming JSON data, expanding its capabilities to interact with modern web applications and services.
  - Integrated `cJSON` library for efficient parsing and handling of JSON data.
  - Implemented new utility functions `process_json_value` and `process_json` to recursively parse JSON objects and arrays.
  - Enhanced the `handleClient` function to detect and process JSON content types (`application/json`).
  - The server can now distinguish between different types of JSON data (strings, numbers, arrays, objects) and print them accordingly.

#### Improvements:
- **HTTP Request Parsing:**
  - Improved the `parse_request` function for more robust parsing of HTTP headers and method detection.
  - Enhanced content length extraction for accurate reading of POST request body data.
  
- **Form Data Handling:**
  - Optimized `process_form_data` function for better handling of URL-encoded form submissions.

#### Bug Fixes:
- Fixed a memory leak issue in the `process_form_data` function where the allocated memory for `rest` was not being freed.
- Addressed a potential buffer overflow vulnerability in the HTTP request parsing logic.

#### Security Enhancements:
- Implemented preliminary checks for content length to prevent potential DoS (Denial of Service) attacks via excessively large POST data submissions.

#### Developer Notes:
- Developers are encouraged to test the new JSON parsing functionality with various JSON structures to ensure compatibility and robustness.
- Feedback on the JSON parsing feature is welcome for continuous improvement in upcoming releases.

#### Acknowledgments:
- Thanks to the contributors who suggested and implemented the JSON parsing feature, enhancing the server's functionality to meet modern web standards.

