#include <assert.h>
#include <cstddef>

#include "row_codec.hpp"

namespace {
    // ENCODER: Packs a vector of bools into a vector of std::byte
    std::vector<std::byte> pack_bools_to_bytes(const std::vector<bool>& bits) {
        std::vector<std::byte> bytes;
        bytes.reserve((bits.size() + 7) / 8);

        unsigned char current_byte = 0;
        int bit_count = 0;

        for (bool bit : bits) {
            if (bit) {
                current_byte |= (1 << bit_count); // LSB-first packing
            }
            bit_count++;

            if (bit_count == 8) {
                bytes.push_back(static_cast<std::byte>(current_byte));
                current_byte = 0;
                bit_count = 0;
            }
        }

        // Add trailing partial byte if necessary
        if (bit_count > 0) {
            bytes.push_back(static_cast<std::byte>(current_byte));
        }

        return bytes;
    }

    // DECODER: Unpacks a vector of std::byte back into a vector of bools
    std::vector<bool> unpack_bytes_to_bools(const std::vector<std::byte>& bytes, size_t original_bit_count) {
        std::vector<bool> bits;
        bits.reserve(original_bit_count);

        size_t processed_bits = 0;

        for (std::byte b : bytes) {
            unsigned char current_byte = std::to_integer<unsigned char>(b);

            for (int i = 0; i < 8; ++i) {
                // Stop parsing as soon as we hit the expected bit count (ignores padding)
                if (processed_bits >= original_bit_count) {
                    return bits;
                }

                // Extract the i-th bit
                bool bit = (current_byte & (1 << i)) != 0;
                bits.push_back(bit);
                processed_bits++;
            }
        }

        return bits;
    }
}


RowCodec::RowCodec(TableSchema schema): schema_{schema} {};

std::vector<std::byte> RowCodec::Encode(const RowValues& row_values) const
{
    std::vector<ColumnDefinition> columns = schema_.GetAllColumns();

    assert(columns.size() == row_values.size() && "RowCodec::Encode requires one value per schema column");
    assert(schema_.GetColumnCount() <= 255 && "Schema definition has 255 or less columns");

    std::vector<std::byte> output;
    std::vector<bool> null_bitmap(schema_.GetColumnCount(), false);

    for (int i = 0; i < columns.size(); i++)
    {
        // Handle null logic
        if (!row_values[i].has_value())
        {
            null_bitmap[i] = true;
            continue;
        }
    }

    // First bytes are always the null_bitmap size, then the null_bitmap
    // The rest is data that is decoded sequentially.
    output.push_back(static_cast<std::byte>(schema_.GetColumnCount()));
    std::vector<std::byte> null_bytes = pack_bools_to_bytes(null_bitmap);
    output.insert(output.end(), null_bytes.begin(), null_bytes.end());

    for (int i = 0; i < columns.size(); i++)
    {
        const ColumnType& column_type = schema_.GetColumn(i).type;

        // Skip Null
        if (null_bitmap[i]) continue;

        auto value = row_values[i].value();
        switch (column_type)
        {
            case ColumnType::BIT: {
                bool v = std::any_cast<bool>(value);
                output.push_back(static_cast<std::byte>(v));
                break;
            }

            default:
                break;
        }
    }

    return output;
}
