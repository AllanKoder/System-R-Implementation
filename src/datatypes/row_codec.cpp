#include <assert.h>
#include <cstddef>
#include <cstdint>

#include "row_codec.hpp"

namespace {
    void append_big_endian(std::vector<std::byte>& output, uint64_t value, size_t byte_count)
    {

    }
}

RowCodec::RowCodec(TableSchema schema): schema_{std::move(schema)} {};

std::vector<std::byte> RowCodec::Encode(const RowValues& row_values) const
{
    const std::vector<ColumnDefinition>& columns = schema_.GetAllColumns();
    const size_t column_count = columns.size();

    assert(columns.size() == row_values.size() && "RowCodec::Encode requires one value per schema column");
    assert(schema_.GetColumnCount() <= 255 && "Schema definition has 255 or less columns");

    std::vector<std::byte> output;
    std::vector<std::byte> null_bytes((column_count + 7) / 8);

    for (size_t i = 0; i < column_count; i++)
    {
        // Handle null logic
        if (!row_values[i].has_value())
        {
            const size_t byte_index = i / 8;
            const size_t bit_index = i % 8;
            unsigned int bitmasking_byte = std::to_integer<unsigned int>(null_bytes[byte_index]);
            null_bytes[byte_index] = static_cast<std::byte>(bitmasking_byte | (1u << bit_index));
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

        const auto& value = row_values[i].value();
        switch (column_type)
        {
            case ColumnType::BIT: {
                bool v = std::any_cast<bool>(value);
                output.push_back(static_cast<std::byte>(v));
                break;
            }
            case ColumnType::TINYINT: {
                int8_t v = std::any_cast<int8_t>(value);
                output.push_back(static_cast<std::byte>(v));
                break;
            }
            case ColumnType::SMALLINT: {
                const auto v = static_cast<uint16_t>(std::any_cast<int16_t>(value));
                append_big_endian(output, v, sizeof(v));
                break;
            }
            case ColumnType::INT: {
                const auto v = static_cast<uint32_t>(std::any_cast<int32_t>(value));
                append_big_endian(output, v, sizeof(v));
                break;
            }
            case ColumnType::BIGINT: {
                const auto v = static_cast<uint64_t>(std::any_cast<int64_t>(value));
                append_big_endian(output, v, sizeof(v));
                break;
            }




            default:
                break;
        }
    }

    return output;
}
