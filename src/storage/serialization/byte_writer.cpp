#include "storage/serialization/byte_writer.hpp"

#include <bit>
#include <climits>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
    constexpr std::uint32_t kCanonicalFloatNaNBits = 0x7FC00000u;
    constexpr std::uint64_t kCanonicalDoubleNaNBits = 0x7FF8000000000000ull;
}

ByteWriter::ByteWriter(std::vector<std::byte>& output) : output_{output} {}

void ByteWriter::write_bytes(std::span<const std::byte> bytes)
{
    if (!bytes.empty())
    {
        output_.insert(output_.end(), bytes.begin(), bytes.end());
    }
}

void ByteWriter::write_unsigned(std::uint64_t value, std::size_t count)
{
    static_assert(CHAR_BIT == 8, "ByteWriter requires 8-bit bytes");
    if (count > sizeof(value))
    {
        throw std::invalid_argument("ByteWriter integer must contain between 0 and 8 bytes");
    }

    for (std::size_t byte_index = 0; byte_index < count; ++byte_index)
    {
        const std::size_t shift = 8 * (count - byte_index - 1);
        output_.push_back(static_cast<std::byte>((value >> shift) & 0xFFu));
    }
}

void ByteWriter::write_float(float value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);
    const std::uint32_t bits = std::isnan(value)
        ? kCanonicalFloatNaNBits
        : std::bit_cast<std::uint32_t>(value);
    write_unsigned(bits, sizeof(bits));
}

void ByteWriter::write_double(double value)
{
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    static_assert(std::numeric_limits<double>::is_iec559);
    const std::uint64_t bits = std::isnan(value)
        ? kCanonicalDoubleNaNBits
        : std::bit_cast<std::uint64_t>(value);
    write_unsigned(bits, sizeof(bits));
}
