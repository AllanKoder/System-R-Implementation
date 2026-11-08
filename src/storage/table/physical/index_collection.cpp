#include "storage/serialization/row_codec.hpp"
#include "storage/table/physical/index_collection.hpp"
#include "lmdb/transaction.hpp"

IndexCollection::IndexCollection(DbInstance& instance, const IndexSchema& schema):
instance_(instance), schema_(schema) { };

void IndexCollection::Insert(RowValues row)
{
    Txn txn(instance_.GetEnv());
    // mdb_put(txn.Get(), instance_.GetEnv(), );
    txn.Commit();
};

void IndexCollection::Edit(RowValueKey key, RowValues row){};

void IndexCollection::Delete(RowValueKey key){};

void IndexCollection::Open(std::optional<RowValueKey> low, std::optional<RowValueKey> high)
{

}

std::optional<RowValue> IndexCollection::GetNext()
{

}

void IndexCollection::Close() noexcept
{

}
