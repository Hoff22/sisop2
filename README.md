# SISOP2 - Project Usage Guide

## Overview

This project implements a concurrent UDP-based client-server system.
It ensures reliable communication over an unreliable protocol (UDP) with timeout, retry, and ordered request handling.

---

smallchange2

## How to Compile

Use the provided `Makefile`.

### Debug Mode (recommended for testing, for performance use release)
```bash
make debug
```

This will generate the executables at:
```
/bin/debug/server
/bin/debug/client
```

### Clean Build Artifacts
```bash
make clean
```

---

## How to Execute the Server

Run the server with:

```bash
./bin/debug/server <port>
```

**Example:**
```bash
./bin/debug/server 4000
```

On startup, the server prints:
```
YYYY-MM-DD HH:MM:SS num_reqs 0 total_sum 0
```

It then starts listening for client discovery and requests.

---

## How to Execute the Client

Run the client with:

```bash
./bin/debug/client <port>
```

**Example:**
```bash
./bin/debug/client 4000
```

Where `<port>` is the same UDP port the server is using.

On startup, the client:
- Broadcasts a discovery packet to find the server.
- Prints the discovered server IP:

```
YYYY-MM-DD HH:MM:SS server_addr <server_ip>
```

After discovery, the client waits for you to input integers:
- Type an integer and press ENTER.
- The client sends the number to the server and waits for acknowledgment.
- Retransmits automatically if no response within the defined timeout in UdpSocket.cpp.

After acknowledgment, the client prints:
```
YYYY-MM-DD HH:MM:SS server <server_ip> id_req <seqn> value <value> num_reqs <total_requests> total_sum <sum>
```

---

## Important Notes

- Use the **same port** for both server and client.
- Only start **one server** in the network.
- You can start **multiple clients**.
- Server processes each request concurrently with worker threads.

---

# Example

In terminal 1:
```bash
./bin/debug/server 4000
```

In terminal 2:
```bash
./bin/debug/client 4000
```

User types:
```
10
7
3
```

Terminal outputs:
```
2025-04-28 12:00:01 server_addr 192.168.0.10
2025-04-28 12:00:02 server 192.168.0.10 id_req 1 value 10 num_reqs 1 total_sum 10
2025-04-28 12:00:03 server 192.168.0.10 id_req 2 value 7 num_reqs 2 total_sum 17
2025-04-28 12:00:04 server 192.168.0.10 id_req 3 value 3 num_reqs 3 total_sum 20
```
