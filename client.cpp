#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

int main() {
    // 1. Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // 2. Specify server address (localhost port 1234)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    // 3. Connect to server
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }
    std::cout << "Connected to server successfully!\n";

    // 4. Send a message
    const char *msg = "Hello from client";
    write(sock_fd, msg, strlen(msg));

    // 5. Read reply
    char buffer[64] = {};
    ssize_t n = read(sock_fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        std::cerr << "Read failed\n";
    } else {
        std::cout << "Server replied: " << buffer << "\n";
    }

    close(sock_fd);
    return 0;
}