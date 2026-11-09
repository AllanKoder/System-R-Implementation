#include <optional>

#include "lmdb.h"
#include "lmdb/transaction.hpp"

#include "main/db_instance.hpp"
#include "storage/table/index_entry.hpp"
#include "storage/table/index_schema.hpp"
#include "storage/serialization/row_codec.hpp"
#include "query/iterator/row_iterator.hpp"


// Represents the physical IndexCollection, holding the actual LMDB data.
// Needs references to the Database instance.
class IndexCollection final : RowIterator {
public:
    //
    explicit IndexCollection(DbInstance& instance, const IndexSchema& schema);

    void Insert(const IndexEntry& entry, std::optional<std::reference_wrapper<Txn>> parent_txn);
    void Edit(const IndexEntry& old_entry, const IndexEntry& new_entry, std::optional<std::reference_wrapper<Txn>> parent_txn);
    void Delete(const IndexEntry& entry, std::optional<std::reference_wrapper<Txn>> parent_txn);

    // Setup for Scan
    void Open(std::optional<RowValueKey> low = std::nullopt, std::optional<RowValueKey> high = std::nullopt);
    std::optional<RowValue> GetNext();
    void Close() noexcept;
private:
    DbInstance& instance_;
    const IndexSchema& schema_;

    MDB_cursor *cursor_;
};
