#include <optional>

#include "lmdb.h"

#include "main/db_instance.hpp"
#include "storage/table/index_schema.hpp"
#include "storage/serialization/row_codec.hpp"
#include "query/iterator/row_iterator.hpp"


// Represents the physical IndexCollection, holding the actual LMDB data.
// Needs references to the Database instance.
class IndexCollection final : RowIterator {
public:
    //
    explicit IndexCollection(DbInstance& instance, const IndexSchema& schema);

    void Insert(RowValues row);
    void Edit(RowValueKey key, RowValues row);
    void Delete(RowValueKey key);

    // Setup for Scan
    void Open(std::optional<RowValueKey> low = std::nullopt, std::optional<RowValueKey> high = std::nullopt);
    std::optional<RowValue> GetNext();
    void Close() noexcept;
private:
    DbInstance& instance_;
    const IndexSchema& schema_;

    MDB_cursor *cursor_;
};
