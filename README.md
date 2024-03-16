# Port Scanner

```shell
g++ -std=c++17 -o scanner -O3 src/*.cpp -pthread -fcoroutines`
```

## TODO
- [x] TCP Connect Scan
- [ ] Error Handling if Host is not reachable
- [ ] Scan IP Range
- [x] epoll
- [ ] Coroutines
- [ ] TCP SYN Scan
- [ ] TCP ACK Scan
- [ ] TCP FIN Scan
- [ ] UDP Scan