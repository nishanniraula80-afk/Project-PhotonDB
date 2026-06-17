# 🚀 Build Your Own Redis

A from-scratch implementation of a low-level, single-threaded, concurrent Key-Value store modeled after Redis. This project explores advanced socket programming, non-blocking I/O event loops, and custom-built core data structures in C/C++.

---

## 🛠️ Environment & Prerequisites

Before diving into the codebase, ensure your development environment meets the following requirements:

* **Language:** C/C++ (Requires a strong understanding of pointers, manual memory management, and low-level data manipulation).
* **OS Environment:** Linux or a Unix-like environment (**macOS** or **WSL on Windows**). The codebase relies heavily on the native Linux socket API.
* **Official Source Code:** You can download the book's reference source code archive [here](https://build-your-own.org/redis/src.tgz) to test or compare against this implementation.

---

## 🗺️ Development Roadmap

The project is divided into three distinct architectural phases, moving from a blank file to a high-performance network server.

### 📍 Phase 1: Environment Setup
- [x] Choose and configure the C/C++ development environment.
- [x] Set up Linux/Unix socket API access.
- [x] Download and archive reference material.

### 📍 Phase 2: Redis from 0 to 1 (Core Architecture)
This phase transforms an empty file into a functioning, asynchronous network server.

* **Socket Programming & TCP:** Implementing a foundational TCP server and client architecture capable of localized network communication.
* **Request-Response Protocol:** Designing a custom wire protocol for safe command parsing and structured client-server communication.
* **Event Loop & Concurrent I/O:** Transitioning away from blocking I/O by implementing a non-blocking **Event Loop** (using `poll` or `epoll`). This allows a single-threaded server to handle thousands of concurrent connections efficiently.
* **In-Memory Key-Value Server:** Building a bare-bones storage engine that maps string keys to values in memory.

### 📍 Phase 3: Advanced Topics (Optimization & Production Features)
Optimizing performance by replacing standard libraries with high-efficiency data structures built entirely from scratch.

| Feature / Module | Description | Implementation Details |
| :--- | :--- | :--- |
| **Custom Hashtables** | Core fast lookups | Dynamic hashtable featuring incremental resizing and rehashing to prevent server blocking. |
| **Data Serialization** | Wire & Disk storage | Custom serialization logic to encode and decode structures over the network. |
| **Sorted Sets (ZSET)** | Range & Score queries | A custom-built Balanced Binary Tree (or skip list) to manage ordered data. |
| **Timers & TTL** | Cache Management | Time-To-Live expiration system that automatically evicts expired keys. |
| **Thread Pool** | Background Workers | Controlled multithreading to offload heavy operations (like disk writes) without lagging the main event loop. |

---

## ⚡ Quick Start

### 1. Clone the repository
```bash
git clone [https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git)
cd YOUR_REPO_NAME
