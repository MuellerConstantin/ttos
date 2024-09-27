#!/usr/bin/env python3

"""
Script to generate a binary initrd image from a list of files.
"""

import os
import sys
import argparse
import struct

INITRD_HEADER_MAGIC = 0xCAFE
INITFS_HEADER_MAGIC = 0xDEAD
INITFS_FILE_HEADER_MAGIC = 0xBEEF

class InitrdHeader:
    def __init__(self) -> None:
        self.magic = INITRD_HEADER_MAGIC

    @staticmethod
    def size() -> int:
        return struct.calcsize("<H")

    def pack(self) -> bytes:
        return struct.pack("<H", self.magic)

class InitfsHeader:
    def __init__(self, file_count: int) -> None:
        self.magic = INITFS_HEADER_MAGIC
        self.file_count = file_count

    @staticmethod
    def size() -> int:
        return struct.calcsize("<HI")

    def pack(self) -> bytes:
        return struct.pack("<HI", self.magic, self.file_count)

class InitfsFileHeader:
    def __init__(self, name: str, offset: int, length: int) -> None:
        if(len(name) > 64):
            raise ValueError("File name is too long.")

        self.magic = INITFS_FILE_HEADER_MAGIC
        self.name = name
        self.offset = offset
        self.length = length

    @staticmethod
    def size() -> int:
        return struct.calcsize("<H64sII")

    def pack(self) -> bytes:
        name_encoded = self.name.encode("ascii") + b"\0" * (64 - len(self.name))
        return struct.pack("<H64sII", self.magic, name_encoded, self.offset, self.length)

def main() -> int:
    parser = argparse.ArgumentParser(prog="mkinitrd", description="Generates a binary initrd image")
    parser.add_argument("-o", "--output", type=str, help="Binary output file.", default="initrd.img")
    parser.add_argument("files", type=str, nargs="*", help="Files to include in the initrd.")

    args = parser.parse_args()

    with open(args.output, "wb") as image:
        initrd_header = InitrdHeader()
        image.write(initrd_header.pack())

        initfs_header = InitfsHeader(len(args.files))
        image.write(initfs_header.pack())

        offset = InitfsHeader.size() + len(args.files) * InitfsFileHeader.size()

        for file in args.files:
            file_size = os.path.getsize(file)
            file_header = InitfsFileHeader(os.path.basename(file), offset, file_size)
            image.write(file_header.pack())

            offset += file_size

        for file in args.files:
            with open(file, "rb") as entry:
                image.write(entry.read())

    return 0

if __name__ == "__main__":
    sys.exit(main())
