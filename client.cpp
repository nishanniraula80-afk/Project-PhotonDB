#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>
#include <cerrno>   // Added for errno
#include <string>

// Helper function to read EXACTLY n bytes from a socket
bool read_all(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv < 0) {
            std::cerr << "Read system call error: " << strerror(errno) << "\n";
            return false;
        }
        if (rv == 0) {
            std::cerr << "Unexpected EOF (Server closed connection prematurely)\n";
            return false; 
        }
        buf += rv;
        n -= (size_t)rv;
    }
    return true;
}

// Helper function to write EXACTLY n bytes to a socket
bool write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            std::cerr << "Write system call error: " << strerror(errno) << "\n";
            return false;
        }
        buf += rv;
        n -= (size_t)rv;
    }
    return true;
}

int main() {
    // 1. Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << "\n";
        return 1;
    }

    // 2. Specify server address (localhost port 1234)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);                   // Port 1234
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    // 3. Connect to server with explicit error diagnostic
    std::cout << "Attempting to connect to server on port 1234...\n";
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << "\n";
        close(sock_fd);
        return 1;
    }
    std::cout << "Connected to server successfully!\n";

    // 4. Test payload pipeline
    std::vector<std::string> messages = {"Ping", "Set key 404", "Get key"};

    for (const auto& msg : messages) {
        uint32_t len = msg.length();

        std::cout << "Sending: " << msg << " (Length: " << len << ")\n";

        // Write header then payload
        if (!write_all(sock_fd, (char*)&len, 4) || !write_all(sock_fd, msg.c_str(), len)) {
            std::cerr << "Failed to send complete message frame.\n";
            break;
        }

        // Read response header
        uint32_t res_len = 0;
        if (!read_all(sock_fd, (char*)&res_len, 4)) {
            std::cerr << "Failed to read response length header.\n";
            break;
        }

        // Read response body payload
        std::vector<char> res_body(res_len + 1, 0);
        if (!read_all(sock_fd, res_body.data(), res_len)) {
            std::cerr << "Failed to read response body payload.\n";
            break;
        }

        std::cout << "Server replied: " << res_body.data() << "\n\n";
        sleep(1);
    }

    std::cout << "Closing client.\n";
    close(sock_fd);
    return 0;
}