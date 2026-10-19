#include "storage/serialization/byte_reader.hpp"

#include <bit>
#include <climits>
#include <limits>
#include <stdexcept>

ByteReader::ByteReader(std::span<const std::byte> bytes) : remaining_{bytes} {}

std::span<const std::byte> ByteReader::read_bytes(std::size_t count)
{
    if (count > remaining_.size())
    {
        throw std::invalid_argument("ByteReader encountered truncated input");
    }

    const auto bytes = remaining_.first(count);
    remaining_ = remaining_.subspan(count);
    return bytes;
}

std::uint64_t ByteReader::read_unsigned(std::size_t count)
{
    static_assert(CHAR_BIT == 8, "ByteReader requires 8-bit bytes");
    if (count > sizeof(std::uint64_t))
    {
        throw std::invalid_argument("ByteReader integer must contain between 0 and 8 bytes");
    }

    std::uint64_t value = 0;
    for (const std::byte byte : read_bytes(count))
    {
        value = (value << 8) | std::to_integer<std::uint8_t>(byte);
    }
    return value;
}

float ByteReader::read_float()
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);
    const auto bits = static_cast<std::uint32_t>(read_unsigned(sizeof(std::uint32_t)));
    return std::bit_cast<float>(bits);
}

double ByteReader::read_double()
{
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    static_assert(std::numeric_limits<double>::is_iec559);
    return std::bit_cast<double>(read_unsigned(sizeof(std::uint64_t)));
}

bool ByteReader::empty() const
{
    return remaining_.empty();
}
