#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "threadpool.h"

constexpr int NUM_THREADS = 16;
constexpr int NUM_PORTS = 65535;
constexpr int TIMEOUT_SECONDS = 1;

void scanPort(const std::string &ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "Error setting socket timeout" << std::endl;
        close(sockfd);
        return;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
        std::cout << "Port " << port << " is open on " << ip << std::endl;
    }

    close(sockfd);
}

int main() {
    std::vector<std::future<void>> futures;
    ThreadPool pool;

    // IP address to scan
    std::string ip = "127.0.0.1";

    for (int port = 1; port <= NUM_PORTS; ++port) {
        futures.emplace_back(pool.enqueue(scanPort, ip, port));
    }

    // Wait for all tasks to complete
    for (auto &future : futures) {
        future.wait();
    }

    return 0;
}
