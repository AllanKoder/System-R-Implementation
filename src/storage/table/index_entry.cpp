#include "storage/table/index_entry.hpp"

#include <utility>

IndexEntry::IndexEntry(RowValueKey key, RowValueKey id):
    key_{std::move(key)}, id_{std::move(id)}
{
}

const RowValueKey& IndexEntry::GetKey() const noexcept
{
    return key_;
}

const RowValueKey& IndexEntry::GetId() const noexcept
{
    return id_;
}
