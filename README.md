# Port Scanner

```shell
g++ -std=c++17 -o scanner -O3 src/*.cpp -pthread -fcoroutines
```

## TODO
- [x] Basic framework
- [x] Basic static thread Pool
- [x] Error Handling if Host is not reachable
- [ ] Invalid arguments handling
- [x] Scan port range
- [x] Scan IP Range
- [ ] Multithreading between different IPs
- [x] epoll
- [x] CPU binding (Seems useless in fact lol)
- [ ] Dynamic thread pool
- [ ] Coroutines (Maybe)
- [x] TCP Connect Scan
- [ ] TCP SYN Scan
- [ ] TCP ACK Scan
- [ ] TCP FIN Scan
- [ ] UDP Scan
- [ ] IPv6 Support