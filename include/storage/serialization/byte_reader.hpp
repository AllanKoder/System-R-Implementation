#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

class ByteReader
{
public:
    explicit ByteReader(std::span<const std::byte> bytes);

    std::span<const std::byte> read_bytes(std::size_t count);
    std::uint64_t read_unsigned(std::size_t count);
    float read_float();
    double read_double();
    bool empty() const;

private:
    std::span<const std::byte> remaining_;
};
