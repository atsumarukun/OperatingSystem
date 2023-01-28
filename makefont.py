#!/usr/bin/python3

import re
import sys
import functools

BITMAP_PATTERN = re.compile(r'([.*@]+)')

def compile(src):
    src = src.lstrip()
    result = []

    for line in src.splitlines():
        m = BITMAP_PATTERN.match(line)
        if not m:
            continue

        bits = [(0 if x == '.' else 1) for x in m.group(1)]
        bits_int = functools.reduce(lambda a, b: 2*a + b, bits)
        result.append(bits_int.to_bytes(1, byteorder='little'))

    return b''.join(result)

def main():
    with open(sys.argv[1], "wb") as out_file, open(sys.argv[2], "r") as in_file:
        src = in_file.read()
        out_file.write(compile(src))

if __name__ == "__main__":
    main()