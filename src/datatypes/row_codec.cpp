#include <assert.h>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "row_codec.hpp"

namespace {
    void append_big_endian(std::vector<std::byte>& output, std::uint64_t value, size_t byte_count)
    {
        static_assert(CHAR_BIT == 8, "RowCodec requires 8-bit bytes");
        assert(byte_count <= sizeof(value));
        for (size_t i = 0; i < byte_count; i++)
        {
            const size_t shift = 8 * (byte_count - i - 1);
            std::byte byte_value = static_cast<std::byte>((value >> shift) & 0xFFu);
            output.push_back(byte_value);
        }
    }

    std::uint64_t read_big_endian(std::vector<std::byte>::iterator begin, std::vector<std::byte>::iterator end)
    {
        static_assert(CHAR_BIT == 8, "RowCodec requires 8-bit bytes");

        // Read Big Endian, first bytes are larger, keep shifting by 8 bits.
        uint64_t output = 0;
        for (auto it = begin; it != end; it++)
        {
            output = (output << 8) |  std::to_integer<uint8_t>(*it);
        }
        return output;
    }

    template <typename T>
    const T& require_value(const RowValue& value, size_t column_index)
    {
        if (const T* typed_value = std::get_if<T>(&value)) return *typed_value;
        throw std::invalid_argument(
            "RowCodec::Encode value does not match schema type at column " + std::to_string(column_index));
    }
}

RowCodec::RowCodec(TableSchema schema): schema_{std::move(schema)} {};

std::vector<std::byte> RowCodec::Encode(const RowValues& row_values) const
{
    const std::vector<ColumnDefinition>& columns = schema_.GetAllColumns();
    const size_t column_count = columns.size();

    assert(columns.size() == row_values.size()); // RowCodec::Encode requires one value per schema column
    assert(schema_.GetColumnCount() <= 255); // Schema definition has 255 or less columns

    std::vector<std::byte> output;
    std::vector<std::byte> null_bytes((column_count + 7) / 8);

    for (size_t i = 0; i < column_count; i++)
    {
        // Handle null logic
        if (std::holds_alternative<std::monostate>(row_values[i]))
        {
            if (!columns[i].nullable)
            {
                throw std::invalid_argument("RowCodec::Encode received NULL for a non-nullable column");
            }
            const size_t byte_index = i / 8;
            const size_t bit_index = i % 8;
            unsigned int current_byte = std::to_integer<unsigned int>(null_bytes[byte_index]);
            null_bytes[byte_index] = static_cast<std::byte>(current_byte | (1u << bit_index));
        }
    }

    // First bytes are always the null_bitmap size, then the null_bitmap
    // The rest is data that is decoded sequentially.
    output.push_back(static_cast<std::byte>(null_bytes.size()));
    output.insert(output.end(), null_bytes.begin(), null_bytes.end());

    for (size_t i = 0; i < column_count; i++)
    {
        const ColumnType& column_type = columns[i].type;

        // Skip Null
        const unsigned int bitmap_byte = std::to_integer<unsigned int>(null_bytes[i / 8]);
        if ((bitmap_byte & (1u << (i % 8))) != 0) continue;

        const RowValue& value = row_values[i];
        switch (column_type)
        {
            case ColumnType::BIT: {
                output.push_back(static_cast<std::byte>(require_value<bool>(value, i)));
                break;
            }
            case ColumnType::TINYINT: {
                output.push_back(static_cast<std::byte>(require_value<std::int8_t>(value, i)));
                break;
            }
            case ColumnType::SMALLINT: {
                const auto v = static_cast<std::uint16_t>(require_value<std::int16_t>(value, i));
                append_big_endian(output, v, sizeof(v));
                break;
            }
            case ColumnType::INT: {
                const auto v = static_cast<std::uint32_t>(require_value<std::int32_t>(value, i));
                append_big_endian(output, v, sizeof(v));
                break;
            }
            case ColumnType::BIGINT: {
                const auto v = static_cast<std::uint64_t>(require_value<std::int64_t>(value, i));
                append_big_endian(output, v, sizeof(v));
                break;
            }
            default:
                throw std::invalid_argument("RowCodec::Encode does not support this column type yet");
        }
    }

    return output;
}
