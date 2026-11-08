#include "storage/table/table_schema.hpp"
#include <string>

// Type is always a B-Tree
// I don't want to deal with composite indexes, so, we will only allow a single column to key.
struct IndexSchema
{
public:
    explicit IndexSchema(const TableSchema& table, std::string column, std::string name, bool ascending = true);

    std::string name;
    std::string column;
    bool ascending;
    const TableSchema& tableSchema;
};
