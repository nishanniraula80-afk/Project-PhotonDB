#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>

bool read_all(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) return false; 
        buf += rv;
        n -= (size_t)rv;
    }
    return true;
}

bool write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) return false;
        buf += rv;
        n -= (size_t)rv;
    }
    return true;
}

void handle_client(int client_fd) {
    const size_t K_MAX_MSG_SIZE = 65536; 

    while (true) {
        uint32_t len = 0;
        // Read 4-byte header
        if (!read_all(client_fd, (char*)&len, 4)) {
            std::cout << "Client disconnected.\n";
            break;
        }

        if (len > K_MAX_MSG_SIZE) {
            std::cerr << "Message too long: " << len << " bytes\n";
            break;
        }

        // Read payload
        std::vector<char> body(len + 1, 0); 
        if (!read_all(client_fd, body.data(), len)) {
            std::cerr << "Read error on body.\n";
            break;
        }

        std::cout << "Client says: " << body.data() << "\n";

        // Send response header + payload
        std::string reply = "ACK: " + std::string(body.data());
        uint32_t res_len = reply.length();

        if (!write_all(client_fd, (char*)&res_len, 4) ||
            !write_all(client_fd, reply.c_str(), res_len)) {
            std::cerr << "Write error.\n";
            break;
        }
    }
    close(client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return 1;

    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return 1;
    if (listen(server_fd, SOMAXCONN) < 0) return 1;

    std::cout << "Server listening on port 1234...\n";

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &socklen);
        if (client_fd < 0) continue;
        
        std::cout << "Client connected!\n";
        handle_client(client_fd);
    }
    close(server_fd);
    return 0;
}