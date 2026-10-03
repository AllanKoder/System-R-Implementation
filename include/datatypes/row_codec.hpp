#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "table_schema.hpp"

// One row in schema column order. std::monostate represents SQL NULL.
using RowValue = std::variant<
    std::monostate,
    bool,
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t,
    float,
    double,
    std::string
>;

using RowValues = std::vector<RowValue>;


class RowCodec
{
public:
    explicit RowCodec(TableSchema schema);

    std::vector<std::byte> Encode(const RowValues& row_values) const;
    RowValues Decode(const std::vector<std::byte>& bytes) const;

private:
    TableSchema schema_;
};
