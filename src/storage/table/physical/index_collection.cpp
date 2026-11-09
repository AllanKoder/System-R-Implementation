#include "storage/serialization/row_codec.hpp"
#include "storage/table/physical/index_collection.hpp"
#include "lmdb/transaction.hpp"

IndexCollection::IndexCollection(DbInstance& instance, const IndexSchema& schema):
instance_(instance), schema_(schema) { };

void IndexCollection::Insert(const IndexEntry& entry, std::optional<std::reference_wrapper<Txn>> parent_txn)
{
    Txn txn(instance_.GetEnv(), parent_txn);
    mdb_put(txn.Get(), instance_.GetEnv(), entry.GetKey(), entry.GetId());
};

void IndexCollection::Edit(const IndexEntry& old_entry, const IndexEntry& new_entry, std::optional<std::reference_wrapper<Txn>> parent_txn){};

void IndexCollection::Delete(const IndexEntry& entry, std::optional<std::reference_wrapper<Txn>> parent_txn){};


void IndexCollection::Open(std::optional<RowValueKey> low, std::optional<RowValueKey> high)
{

}

std::optional<RowValue> IndexCollection::GetNext()
{

}

void IndexCollection::Close() noexcept
{

}
