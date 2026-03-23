# What is it?

This project is an implementation of high-performance concurrent KV-store server using C++20. The server takes in `SET`, `GET`, and `DEL` requests formatted under the RESP (Redis Serialization Protocol) protocol and perform corresponding operations. Under the testing of memtier, the server by far is able to achieve an average of 153,000+ ops/sec, with a p99 of 1.96 ms.

# Tech Stack

- C++ with Makefile

- Linux epoll

- Docker

- GitHub Actions

- AWS EC2

# Components

- Memory Pool (thread-local): the raw memory chunk and memory allocator

- Hash Table: the interface of the KV-store

- Thread Pool: manages threads and their tasks

- Server: Linux epoll server

- Logger: logs debug info

# Build & Test

To build the server only, run

```bash
make server
```

To build all the tests, run

```bash
make test
```

To build everything, simply run

```bash
make
```

The server can be run under the project root directory:

```bash
./server
```

All the tests are located in the `/test` folder.



*For a deep dive into the engineering decisions and memory management architecture, read the full case study [here](https://yi-zh17.github.io/posts/kv-store/)*
