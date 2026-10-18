#include <assert.h>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "byte_reader.hpp"
#include "byte_writer.hpp"
#include "row_codec.hpp"

// NOTE: This is a very slow, and unoptimized serilization/encoding format. This is more for learning purposes than anything else!
namespace {
    template <typename T>
    const T& require_value(const RowValue& value, size_t column_index)
    {
        if (const T* typed_value = std::get_if<T>(&value)) return *typed_value;
        throw std::invalid_argument(
            "RowCodec::Encode value does not match schema type at column " + std::to_string(column_index));
    }
}

/*
* Row Tuple Format:
* Byte 0-(Header Size): Null Bytes Header
* Byte Header-End: Encoded Data
*/
RowCodec::RowCodec(TableSchema schema): schema_{std::move(schema)} {};

/*
* Encoding will result in a data format that is unlikely to work on another computer architecture due to how they handle datatypes
* However, it is not important for the data pages to be compatible, this is not SQLite.
* Focus is getting the system to work as close to the hardware level as possible (and as easily as possible).
*/
std::vector<std::byte> RowCodec::Encode(const RowValues& row_values) const
{
    const std::vector<ColumnDefinition>& columns = schema_.GetAllColumns();
    const size_t column_count = columns.size();

    assert(columns.size() == row_values.size()); // RowCodec::Encode requires one value per schema column
    assert(schema_.GetColumnCount() <= 255); // Schema definition has 255 or less columns

    std::vector<std::byte> output;
    ByteWriter writer{output};
    std::vector<std::byte> null_bytes((column_count + 7) / 8);

    // Handle null logic
    for (size_t i = 0; i < column_count; i++)
    {
        if (std::holds_alternative<std::monostate>(row_values[i]))
        {
            if (!columns[i].nullable)
            {
                throw std::invalid_argument("RowCodec::Encode received NULL for a non-nullable column");
            }
            // Create the null butes array
            const size_t byte_index = i / 8;
            const size_t bit_index = i % 8;
            unsigned int current_byte = std::to_integer<unsigned int>(null_bytes[byte_index]);
            null_bytes[byte_index] = static_cast<std::byte>(current_byte | (1u << bit_index));
        }
    }

    // Inserting the null bytes header
    writer.write_bytes(null_bytes);

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
                writer.write_unsigned(require_value<bool>(value, i), 1);
                break;
            }
            case ColumnType::TINYINT: {
                writer.write_unsigned(static_cast<std::uint8_t>(require_value<std::int8_t>(value, i)), 1);
                break;
            }
            case ColumnType::SMALLINT: {
                const auto v = static_cast<std::uint16_t>(require_value<std::int16_t>(value, i));
                writer.write_unsigned(v, sizeof(v));
                break;
            }
            case ColumnType::INT: {
                const auto v = static_cast<std::uint32_t>(require_value<std::int32_t>(value, i));
                writer.write_unsigned(v, sizeof(v));
                break;
            }
            case ColumnType::BIGINT: {
                const auto v = static_cast<std::uint64_t>(require_value<std::int64_t>(value, i));
                writer.write_unsigned(v, sizeof(v));
                break;
            }
            case ColumnType::FLOAT:
            {
                const auto v = require_value<float>(value, i);
                writer.write_float(v);
                break;
            }
            case ColumnType::DOUBLE:
            {
                const auto v = require_value<double>(value, i);
                writer.write_double(v);
                break;
            }
            case ColumnType::CHAR:
            case ColumnType::VARCHAR: {

                const auto& v = require_value<std::string>(value, i);
                if (v.size() > std::numeric_limits<std::uint16_t>::max())
                {
                    throw std::invalid_argument("RowCodec string exceeds 65535 bytes");
                }
                // Add 2 bytes to prefix the length of the string.
                writer.write_unsigned(v.size(), sizeof(std::uint16_t));
                // Append all bytes without care about checking validity
                // Note DuckDB does not care about the specified length: https://duckdb.org/docs/current/sql/data_types/text
                // Why should we care about checking?
                writer.write_bytes(std::as_bytes(std::span{v.data(), v.size()}));
                break;
            }
            default:
                throw std::invalid_argument("RowCodec::Encode does not support this column type yet");
        }
    }

    return output;
}

RowValues RowCodec::Decode(const std::vector<std::byte>& bytes) const
{
    RowValues result;
    const std::vector<ColumnDefinition>& columns = schema_.GetAllColumns();
    const size_t column_count = columns.size();

    assert(schema_.GetColumnCount() <= 255); // Schema definition has 255 or less columns

    result.reserve(column_count);
    ByteReader reader{bytes};
    const auto null_bytes = reader.read_bytes((column_count + 7) / 8);

    for (size_t column_index = 0; column_index < column_count; ++column_index)
    {
        const auto& column = columns[column_index];
        const auto bitmap_byte = std::to_integer<unsigned int>(null_bytes[column_index / 8]);
        if ((bitmap_byte & (1u << (column_index % 8))) != 0)
        {
            if (!column.nullable)
            {
                throw std::invalid_argument("RowCodec::Decode received NULL for a non-nullable column");
            }
            result.emplace_back(std::monostate{});
            continue;
        }

        switch (column.type)
        {
            case ColumnType::BIT: {
                result.emplace_back(static_cast<bool>(reader.read_unsigned(1)));
                break;
            }
            case ColumnType::TINYINT: {
                const auto bits = static_cast<std::uint8_t>(reader.read_unsigned(sizeof(std::uint8_t)));
                result.emplace_back(std::bit_cast<std::int8_t>(bits));
                break;
            }
            case ColumnType::SMALLINT: {
                const auto bits = static_cast<std::uint16_t>(reader.read_unsigned(sizeof(std::uint16_t)));
                result.emplace_back(std::bit_cast<std::int16_t>(bits));
                break;
            }
            case ColumnType::INT: {
                const auto bits = static_cast<std::uint32_t>(reader.read_unsigned(sizeof(std::uint32_t)));
                result.emplace_back(std::bit_cast<std::int32_t>(bits));
                break;
            }
            case ColumnType::BIGINT: {
                result.emplace_back(std::bit_cast<std::int64_t>(reader.read_unsigned(sizeof(std::uint64_t))));
                break;
            }
            case ColumnType::FLOAT:
            {
                result.emplace_back(reader.read_float());
                break;
            }
            case ColumnType::DOUBLE:
            {
                result.emplace_back(reader.read_double());
                break;
            }
            case ColumnType::CHAR:
            case ColumnType::VARCHAR: {
                const auto length = static_cast<size_t>(reader.read_unsigned(sizeof(std::uint16_t)));
                const auto text_bytes = reader.read_bytes(length);
                std::string text;
                if (!text_bytes.empty())
                {
                    text.assign(reinterpret_cast<const char*>(text_bytes.data()), text_bytes.size());
                }
                result.emplace_back(std::move(text));
                break;
            }
            default:
                throw std::invalid_argument("RowCodec::Decode does not support this column type yet");
        }
    }

    if (!reader.empty())
    {
        throw std::invalid_argument("RowCodec::Decode received trailing bytes");
    }
    return result;
}
