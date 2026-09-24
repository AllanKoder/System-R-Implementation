#include "table_schema.hpp"
#include <assert.h>

ColumnDefinition::ColumnDefinition(std::string name, ColumnType type, bool nullable):
name(name), type(type), nullable(nullable) {};

TableSchema::TableSchema(std::vector<ColumnDefinition> fields): fields_{fields} { }

int TableSchema::GetColumnCount() const
{
    return fields_.size();
}

const std::vector<ColumnDefinition>& TableSchema::GetAllColumns() const
{
    return fields_;
}

const ColumnDefinition& TableSchema::GetColumn(std::string name) const
{
    for (const ColumnDefinition& definition : fields_)
    {
        if (definition.name == name) return definition;
    }
    std::__throw_runtime_error("TableSchema::GetColumn name not found");
}
