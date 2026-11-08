#include "storage/table/index_schema.hpp"

IndexSchema::IndexSchema(const TableSchema& tableSchema, std::string column, std::string name, bool ascending):
tableSchema(tableSchema), column(column), name(name), ascending(ascending) {}
