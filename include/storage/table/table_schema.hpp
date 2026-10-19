#pragma once
#include <vector>
#include <string>

#include "storage/table/column_types.hpp"

struct ColumnDefinition {
    explicit ColumnDefinition(std::string name, ColumnType type, bool nullable);

    std::string name;
    ColumnType type;
    bool nullable;
};

class TableSchema
{
public:
    explicit TableSchema(std::vector<ColumnDefinition> fields);

    int GetColumnCount() const;
    const std::vector<ColumnDefinition>& GetAllColumns() const;
    const ColumnDefinition& GetColumn(std::string name) const;

private:
    std::vector<ColumnDefinition> fields_; // max 255 (byte)
};
