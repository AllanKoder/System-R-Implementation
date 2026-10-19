#include "storage/table/table_schema.hpp"

// Holds information relating to the data table (Logical Table)
// TODO: Column Definition Might need some more properties?
// I think properities like primary keys, sorting, what index, etc

// TODO:
// IndexDefinition
class DataTable
{
    explicit DataTable(const TableSchema& schema, );

    // Methods:
    // GetRowCollection -> GetRowCollection (Holds the physical table)
    // GetIndexCollection -> IndexCollection (Holds the physical index)
    // GetIndexDefinitions -> GetIndexes (Collection)
    // Column and Index are both interfaces of IDataCollection
    // GetSchema

    // Properties:
    // Schema, Indexes, Column
    // Catalog -> Tables,

    // Should we include information about the primary index?
    // - No, that is in the index definitions
    // We can also declare the schema in here?
    // - Schema is defined in schema, no need to define anything here. We will create the schema in Catalog (Create Table)
    // - Then, the DataTable will have a reference to the Catalog, and work from there.
};
