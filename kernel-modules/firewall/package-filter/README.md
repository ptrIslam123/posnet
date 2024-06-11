# Net Package Logger

## Overview

This is a simple Linux kernel module that logs all incoming and outgoing IP packages. It uses Netfilter hooks to intercept packets and then parses the IP, TCP, UDP, and ICMP headers to log relevant information.

## Features

- Logs source and destination IP addresses.
- Logs source and destination port numbers for TCP and UDP packets.
- Logs sequence and acknowledgment numbers for TCP packets.
- Logs ICMP type and code for ICMP packets.
- Logs the protocol type and TTL for IP packets.

## Compilation

To compile this module, you need to have the appropriate Linux headers installed on your system. You can compile the module using the following commands:

```bash
# to compile and load 
make && sudo insmod net_log

# to unload this linux module from the system:
sudo rmmod net_log

# to print log information
sudo dmesg
```
