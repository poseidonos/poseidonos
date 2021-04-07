# Poseidon OS

Poseidon OS (POS) is a light-weight storage OS that offers the best performance and valuable features over storage network. POS exploits the benefit of NVMe SSDs by optimizing storage stack and leveraging the state-of-the-art high speed interface. Please find project details at documentations page.

## Source Code

```bash
git clone https://github.com/poseidonos/poseidonos

```

## Prerequisites

This script will automatically install the minimum dependencies required to build Poseidon OS.

```bash
cd script
sudo ./pkgdep.sh
```

## How to build

### 1. Build Library

```bash
cd lib
sudo ./build_ibof_lib.sh all
```

### 2. Build source code

```bash
cd script
sudo ./build_ibofos.sh
```

## Preparation

```bash
cd script/
sudo ./setup_env.sh
```

## Run POS

```bash
sudo ./bin/ibofos
```