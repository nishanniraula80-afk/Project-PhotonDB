#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

int main() {
    // 1. Create a TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Allow immediate reuse of the port after server restarts
    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // 2. Bind the socket to port 1234
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);       // Port 1234
    addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections on any network interface

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // 3. Start listening for incoming connections
    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }
    std::cout << "Server is listening on port 1234...\n";

    // 4. Accept a client connection
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &socklen);
    if (client_fd < 0) {
        std::cerr << "Accept failed\n";
        return 1;
    }
    std::cout << "Client connected!\n";

    // 5. Read data from the client
    char buffer[64] = {};
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        std::cerr << "Read failed\n";
    } else {
        std::cout << "Client says: " << buffer << "\n";
    }

    // 6. Reply to the client
    const char *reply = "Hello from server";
    write(client_fd, reply, strlen(reply));

    // Cleanup
    close(client_fd);
    close(server_fd);
    return 0;
}