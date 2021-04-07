#!/usr/bin/env python3

import random

if __name__ == "__main__":
    success = random.choice([True, False])
    if success:
        result = 0
    else:
        result = 1
    exit(result)
