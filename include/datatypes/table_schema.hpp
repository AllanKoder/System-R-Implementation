#pragma once
#include <vector>
#include <string>

#include "column_types.hpp"

struct ColumnDefinition
{
    std::string name;
    ColumnType type;
    bool nullable;
};

class TableSchema
{
public:
    int GetColumnCount() const;
    std::vector<ColumnDefinition>& GetAllColumns() const;
    ColumnDefinition GetColumn(std::string name) const;

private:
    std::vector<ColumnDefinition> fields;
};
