#include <vector>

#include "threadpool.h"
#include "scanner.h"

int main(int argc, char* argv[]) {
    int threadNum = 48;

    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <ip>" << " <start port>" << " <end port>" << std::endl;
        return 1;
    }
    std::string ip = argv[1];
    int startPort = std::stoi(argv[2]);
    int endPort = std::stoi(argv[3]);
    std::vector<int> ports;
    for (int i = startPort; i <= endPort; i++) {
        ports.push_back(i);
    }

    ThreadPool pool(threadNum);
    for (int i = 0; i < threadNum; i++) {
        // 分段扫描
        std::vector<int> subPorts(ports.begin() + i * (endPort - startPort) / threadNum, ports.begin() + (i + 1) * (endPort - startPort) / threadNum);
        pool.enqueue([i, ip, subPorts] {
            // std::cout << "Task " << i << " is running on core " << sched_getcpu() << std::endl;
            ScannerFactory factory;
            Scanner* scanner = factory.createScanner("TCPConnect");
            scanner->scan(ip, subPorts);
        });
    }
    return 0;
}