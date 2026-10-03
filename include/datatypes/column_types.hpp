#pragma once

// Portable row wire format. Multibyte values are encoded most-significant byte
// first; never serialize a C++ object's in-memory representation directly.
enum class ColumnType
{
    BIT, // bool; one byte, exactly 0x00 or 0x01
    TINYINT, // signed 8-bit two's-complement integer; one byte
    SMALLINT, // signed 16-bit two's-complement integer; 2 bytes, big-endian
    INT, // signed 32-bit two's-complement integer; 4 bytes, big-endian
    BIGINT, // signed 64-bit two's-complement integer; 8 bytes, big-endian
    FLOAT, // IEEE 754 binary32 bits, big-endian; encode NaN as 0x7FC00000
    DOUBLE, // IEEE 754 binary64 bits, big-endian; encode NaN as 0x7FF8000000000000
    CHAR, // UTF-8, schema-defined byte width; space-pad to width, reject overflow
    VARCHAR // UTF-8 bytes prefixed by a 4-byte unsigned big-endian byte length
};
