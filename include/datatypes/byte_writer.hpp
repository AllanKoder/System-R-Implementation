#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

class ByteWriter
{
public:
    explicit ByteWriter(std::vector<std::byte>& output);

    void write_bytes(std::span<const std::byte> bytes);
    void write_unsigned(std::uint64_t value, std::size_t count);
    void write_float(float value);
    void write_double(double value);

private:
    std::vector<std::byte>& output_;
};
