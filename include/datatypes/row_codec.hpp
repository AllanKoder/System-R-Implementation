#pragma once
#include <vector>
#include <string>
#include <optional>
#include <any>

#include "table_schema.hpp"

// One decoded row, in schema column order. A std::nullopt entry means SQL NULL.
// Value types mirror ColumnVector's std::any contents so the two layers stay interchangeable.
using RowValues = std::vector<std::optional<std::any>>;


class RowCodec
{
public:
    explicit RowCodec(TableSchema schema);

    std::string Encode(const RowValues& row_values) const;
    RowValues Decode(const std::string& bytes) const;

private:
    TableSchema schema_;
};
