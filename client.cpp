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

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return 1;

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }
    std::cout << "Connected to server protocol!\n";

    // Array of distinct payload messages to test stream boundaries
    std::vector<std::string> messages = {
        "Ping",
        "Set key_xyz 404",
        "Get key_xyz"
    };

    for (const auto& msg : messages) {
        uint32_t len = msg.length();

        // Send 4-byte header length, then payload data
        if (!write_all(sock_fd, (char*)&len, 4) ||
            !write_all(sock_fd, msg.c_str(), len)) {
            std::cerr << "Write error\n";
            break;
        }

        // Read server response header length
        uint32_t res_len = 0;
        if (!read_all(sock_fd, (char*)&res_len, 4)) {
            std::cerr << "Read error on response header\n";
            break;
        }

        // Read server response body payload
        std::vector<char> res_body(res_len + 1, 0);
        if (!read_all(sock_fd, res_body.data(), res_len)) {
            std::cerr << "Read error on response body\n";
            break;
        }

        std::cout << "Server replied: " << res_body.data() << "\n";
        sleep(1);
    }

    close(sock_fd);
    return 0;
}