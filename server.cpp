#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <cerrno>

const size_t K_MAX_MSG_SIZE = 65536;

// Track the byte buffers for each active connection
struct Conn {
    int fd = -1;
    std::vector<uint8_t> rbuf; // Read buffer
    std::vector<uint8_t> wbuf; // Write buffer
};

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

// Try to process a single request from the read buffer
bool try_one_request(Conn *conn) {
    // We need at least 4 bytes to parse the length header
    if (conn->rbuf.size() < 4) {
        return false; 
    }

    uint32_t len = 0;
    std::memcpy(&len, conn->rbuf.data(), 4);

    if (len > K_MAX_MSG_SIZE) {
        std::cerr << "Message too long: " << len << " bytes\n";
        close(conn->fd);
        conn->fd = -1;
        return false;
    }

    // Do we have the complete payload frame yet?
    if (conn->rbuf.size() < 4 + len) {
        return false; 
    }

    // Extract message body text
    std::string msg((char*)(conn->rbuf.data() + 4), len);
    std::cout << "Async completely parsed: " << msg << "\n";

    // Craft the response frame
    std::string reply = "ACK: " + msg;
    uint32_t res_len = reply.length();

    // Append to write buffer
    size_t cur_wbin = conn->wbuf.size();
    conn->wbuf.resize(cur_wbin + 4 + res_len);
    std::memcpy(conn->wbuf.data() + cur_wbin, &res_len, 4);
    std::memcpy(conn->wbuf.data() + cur_wbin + 4, reply.c_str(), res_len);

    // Erase processed request from read buffer
    conn->rbuf.erase(conn->rbuf.begin(), conn->rbuf.begin() + 4 + len);
    return true;
}

void handle_client_io(Conn *conn) {
    uint8_t buf[4096];
    ssize_t rv = read(conn->fd, buf, sizeof(buf));
    
    if (rv < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        std::cout << "Read system error\n";
        close(conn->fd);
        conn->fd = -1;
        return;
    }
    
    if (rv == 0) {
        std::cout << "Client closed connection context cleanly.\n";
        close(conn->fd);
        conn->fd = -1;
        return;
    }

    // Append new data chunks into the connection state array
    conn->rbuf.insert(conn->rbuf.end(), buf, buf + rv);

    // Process as many full frames as possible out of the buffer stream
    while (try_one_request(conn)) {}

    // Flush any pending data out of the write buffer back to the socket pipeline
    if (!conn->wbuf.empty() && conn->fd != -1) {
        ssize_t wv = write(conn->fd, conn->wbuf.data(), conn->wbuf.size());
        if (wv > 0) {
            conn->wbuf.erase(conn->wbuf.begin(), conn->wbuf.begin() + wv);
        }
    }
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

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }
    if (listen(server_fd, SOMAXCONN) < 0) return 1;

    set_nonblocking(server_fd);
    std::cout << "Buffered Asynchronous Server listening on port 1234...\n";

    // Track state structures alongside poll file descriptors
    std::vector<Conn*> fd_to_conn;
    std::vector<struct pollfd> poll_fds;

    struct pollfd server_pfd = {server_fd, POLLIN, 0};
    poll_fds.push_back(server_pfd);
    fd_to_conn.push_back(nullptr); // Index 0 matches the listening socket

    while (true) {
        for (auto &pfd : poll_fds) pfd.revents = 0;

        int rv = poll(poll_fds.data(), (nfds_t)poll_fds.size(), -1);
        if (rv < 0 && errno != EINTR) break;

        // 1. Accept new client connections
        if (poll_fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr = {};
            socklen_t socklen = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &socklen);
            
            if (client_fd >= 0) {
                set_nonblocking(client_fd);
                struct pollfd client_pfd = {client_fd, POLLIN, 0};
                poll_fds.push_back(client_pfd);

                Conn *c = new Conn();
                c->fd = client_fd;
                fd_to_conn.push_back(c);
                std::cout << "Accepted connection. Monitored sockets: " << poll_fds.size() - 1 << "\n";
            }
        }

        // 2. Route events to specific data buffers
        for (size_t i = 1; i < poll_fds.size(); ) {
            if (poll_fds[i].revents & POLLIN) {
                handle_client_io(fd_to_conn[i]);

                // If connection was marked closed inside IO handlers, clear tracking references
                if (fd_to_conn[i]->fd == -1) {
                    delete fd_to_conn[i];
                    poll_fds.erase(poll_fds.begin() + i);
                    fd_to_conn.erase(fd_to_conn.begin() + i);
                    std::cout << "Cleared connection allocation context. Monitored sockets: " << poll_fds.size() - 1 << "\n";
                    continue;
                }
            }
            i++;
        }
    }

    close(server_fd);
    return 0;
}