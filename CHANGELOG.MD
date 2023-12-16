
---

### Changelog for Release 1.7 - 16/12/2023

#### New Features:
- **HTTPS Support:**
  - Integrated OpenSSL to implement HTTPS, enhancing the security of data transmission between the server and clients.
  - The server now supports SSL/TLS encryption, ensuring that all communication is encrypted and secure from eavesdropping and man-in-the-middle attacks.
  - Added functionality to load SSL certificates and private keys, enabling the server to authenticate itself to clients and establish an encrypted channel.

#### Enhancements:
- **SSL/TLS Protocol Optimization:**
  - Implemented SSL context initialization using the latest TLS server methods for improved security.
  - Added automatic ECDH curve selection for better compatibility and performance in SSL/TLS handshakes.
  - Improved error handling and logging in SSL operations for easier debugging and maintenance.

#### Security Improvements:
- **Enhanced Security for Client-Server Communication:**
  - With HTTPS, sensitive data such as login credentials, personal information, and other confidential data are now transmitted securely.
  - Implemented strict SSL cipher suites and protocols to mitigate common SSL/TLS vulnerabilities and attacks.
  
#### Developer Notes:
- **SSL Certificate Management:**
  - Developers need to provide valid SSL certificates and corresponding private keys for deployment. Self-signed certificates can be used for development and testing.
  - Instructions added to the documentation on how to generate and set up SSL certificates.

- **Build and Dependency Changes:**
  - The build process now requires linking with OpenSSL libraries (`-lssl -lcrypto`).
  - Developers must ensure OpenSSL is installed and properly configured in the development environment.

#### Bug Fixes:
- None in this release.

#### Known Issues:
- None reported as of this release.


