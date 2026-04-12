# Zero Allocation KV-Store Server

A high-performance, concurrent key-value store server written in C++20. It speaks the [RESP (Redis Serialization Protocol)](https://redis.io/docs/latest/develop/reference/protocol-spec/) and is designed for predictable low-latency under high concurrency via pre-allocated, thread-local memory pools that eliminate dynamic allocation during steady-state operation.

**Benchmark (memtier, 4 threads, 50 clients):** ~244,000 ops/sec · p99 1.08 ms

---

## Architecture

| Component | Description |
|-----------|-------------|
| **Server** | Linux epoll-based non-blocking event loop, dispatches connections to the thread pool |
| **Thread Pool** | One worker per CPU core; each worker owns a thread-local memory pool |
| **Hash Table** | Flat open-addressing table with Robin Hood hashing and shared-mutex for concurrent reads |
| **Memory Pool** | Thread-local pre-allocated chunks (256 B × 10,000 per thread); returns `nullptr` on exhaustion rather than calling `malloc` |
| **Logger** | Debug-only logging compiled out in release (`-DNDEBUG`) |

Default capacity: **1,000,000** key-value pairs · max key: 64 B · max value: 192 B · port: **8080**

---

## Build & Run

**Requirements:** g++ with C++20 support, Linux (epoll)

```bash
make server   # build server binary
make test     # build all test binaries
make          # build everything
make clean    # remove build artifacts
```

```bash
./server      # starts on port 8080
```

Tests are in [test/](test/). Run each binary directly, e.g. `./test/TestHashTable`.

### Docker

```bash
docker build -t kv-server .
docker run -p 8080:8080 kv-server
```

---

## Protocol

The server accepts RESP-formatted commands:

| Command | Response |
|---------|----------|
| `SET <key> <value>` | `+OK` / `-ERR insertion failed` |
| `GET <key>` | `$<len>\r\n<value>` / `$-1` (nil) |
| `DEL <key>` | `:1` (deleted) / `:0` (not found) |

Quick smoke test:
```bash
redis-cli -p 8080 SET foo bar
redis-cli -p 8080 GET foo
```

*For a deep dive into the engineering decisions and memory management architecture, read the full case study [here](https://yi-zh17.github.io/posts/kv-store/).*
