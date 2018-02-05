# Learning Objective

* Libemu x86 emulation library
* Use-after-free exploitation
* Bugs in emulators/VMs

# Description

The target application uses `libemu` to emulated input data as `X86` machine code while hooking certain interrupts. An attacker can manage to create an `User After Free` condition by carefully crafting memory operations.

# Usage

```
$ docker build -t uniemu .
$ docker run --rm --name uniemu -p 9000:9000 uniemu
```

# Test

```bash
$ ruby -e 'print "\x04\x00\x00\x00\x41\x00\x41\x41"' | nc 127.0.0.1 9000
```


